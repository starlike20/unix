#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <mysql.h>
#include <setjmp.h>
#include "protocole.h" // contient la cle et la structure d'un message
#include "FichierUtilisateur.h"
#include <sys/shm.h>
#include <time.h>
int idQ,idShm,idSem,idShms,id1;
TAB_CONNEXIONS *tab;
sigjmp_buf contexte;

void afficheTab();
void connect(int a,int b);
void logi(int pid,const char*nom);
void loginn(MESSAGE m);
int verificonect(const char*nom);
void HandlerSIGINT(int sig);
void HandlerSIGCHILD(int sig);
void accept(MESSAGE m);
void decon(MESSAGE m);
void refus(MESSAGE m);
void sen(MESSAGE m);
void pube();
void addbd(MESSAGE m);
void ajoutmodif(MESSAGE*m,int idm);

MYSQL* connexion;
struct sembuf action;

void sem(int a,int b);


int main()
{
  int idm,c;
  // Connection à la BD

  // Armement des signaux
  struct sigaction B;
  B.sa_handler = HandlerSIGINT;
  B.sa_flags = 0;
  sigemptyset(&B.sa_mask);
  sigaction(SIGINT,&B,NULL);

  struct sigaction A;
  A.sa_handler = HandlerSIGCHILD;
  A.sa_flags = 0;
  sigemptyset(&A.sa_mask);
  sigaction(SIGCHLD,&A,NULL);

  fprintf(stderr,"(SERVEUR %d) Recuperation de l'id de la mémoire partagée\n",getpid());
  if ((idShms = shmget(123,0,0)) == -1)  
  {
    printf("premier serveur lancer\n");
    if ((idShms = shmget(123,200,IPC_CREAT | IPC_EXCL | 0600)) == -1){
      perror("(SERVEUR) Erreur de shmget");
      exit(1);
    }
    // Creation des ressources
    fprintf(stderr,"(SERVEUR %d) Creation de la file de messages\n",getpid());
    if ((idQ = msgget(CLE,IPC_CREAT | IPC_EXCL | 0600)) == -1)  // CLE definie dans protocole.h
    {
      perror("(SERVEUR) Erreur de msgget");
      exit(1);
    }

    if ((idShm = shmget(CLE,200,IPC_CREAT | IPC_EXCL | 0600)) == -1){
      perror("(SERVEUR) Erreur de shmget");
      exit(1);
    }
    if ((idSem = semget(CLE,2,IPC_CREAT | IPC_EXCL | 0600)) == -1)
    {
      perror("Erreur de semget");
      exit(1);
    }
    if((tab = (TAB_CONNEXIONS*)shmat(idShms,NULL,0))==NULL)
    {
      fprintf(stderr,"(SERVEUR %d) erreur d'attachement",getpid());
    }
    sem(0,1);
    sem(1,1);

    sem(1,-1);

    // Initialisation du tableau de connexions
    fprintf(stderr,"(SERVEUR %d) Initialisation de la table des connexions\n",getpid());
    //tab = (TAB_CONNEXIONS*) malloc(sizeof(TAB_CONNEXIONS)); 

    for (int i=0 ; i<6 ; i++)
    {
      tab->connexions[i].pidFenetre = 0;
      strcpy(tab->connexions[i].nom,"");
      for (int j=0 ; j<5 ; j++) tab->connexions[i].autres[j] = 0;
      tab->connexions[i].pidModification = 0;
    }
    tab->pidServeur1 = getpid();
    tab->pidServeur2 = 0;
    tab->pidAdmin = 0;
    tab->pidPublicite = 0;

    sem(1,1);

    afficheTab();

    // Creation du processus Publicite
    int idp,id;
    /*if((id=fork())!=-1){
      if(id==0){
        if(execl("./Publicite","Publicite",NULL)==-1){
          fprintf(stderr,"(SERVEUR)Erreur de execl");
        }
      }
    }
    sem(1,-1);
    tab->pidPublicite=id;
    sem(1,1);*/
  }
  else{
    if((tab = (TAB_CONNEXIONS*)shmat(idShms,NULL,0))==NULL)
    {
      fprintf(stderr,"(SERVEUR %d) erreur d'attachement",getpid());
    }
    sem(1,-1);
    if(tab->pidServeur1==0 || kill(tab->pidServeur1,0)==-1){
      tab->pidServeur1=getpid();
      if((idQ = msgget(CLE,0))==-1)
      {
        fprintf(stderr,"(SERVEUR %d) erreur de Recuperation de l'id de la file de messages\n",getpid());
      }
      if ((idSem = semget(CLE,0,0)) == -1)
      {
        fprintf(stderr,"(SERVEUR %d)Erreur de semget",getpid());
        exit(1);
      }
      if((idShm = shmget(CLE,0,0)) == -1){
        fprintf(stderr,"(SERVEUR %d) shmget\n",getpid());
      }
    }
    else{ 
      if(tab->pidServeur2==0 || kill(tab->pidServeur2,0)==-1){
        tab->pidServeur2=getpid();
        if((idQ = msgget(CLE,0))==-1)
        {
          fprintf(stderr,"(SERVEUR %d) erreur de Recuperation de l'id de la file de messages\n",getpid());
        }
        if ((idSem = semget(CLE,0,0)) == -1)
        {
          fprintf(stderr,"(SERVEUR %d)Erreur de semget",getpid());
          exit(1);
        }
        if((idShm = shmget(CLE,0,0)) == -1){
          fprintf(stderr,"(SERVEUR %d) shmget\n",getpid());
        }
      }
      else{
        if (shmdt(tab) == -1) {
          perror("Erreur de shmdt");
        }
        printf("deux serveur sont deja lancer \n");
        exit(0);
      }
    }
    sem(1,1);
  }

  int i,k,j,tr;
  MESSAGE m;
  
  MESSAGE reponse;
  char requete[200];
  MYSQL_RES  *resultat;
  MYSQL_ROW  tuple;
  PUBLICITE pub;

  while(1)
  {
    sigsetjmp(contexte,1);
  	fprintf(stderr,"(SERVEUR %d) Attente d'une requete...\n",getpid());
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),1,0) == -1)
    {
      perror("(SERVEUR) Erreur de msgrcv");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
    }

    switch(m.requete)
    {
      case CONNECT :  
                      fprintf(stderr,"(SERVEUR %d) Requete CONNECT reçue de %d\n",getpid(),m.expediteur);
                      sem(1,-1);
                      connect(m.expediteur,0);
                      sem(1,1);
                      break; 

      case DECONNECT :  
                      fprintf(stderr,"(SERVEUR %d) Requete DECONNECT reçue de %d\n",getpid(),m.expediteur);
                      sem(1,-1);
                      logi(m.expediteur,"");
                      connect(0,m.expediteur);
                      sem(1,1);
                      break; 

      case LOGIN :  
                      fprintf(stderr,"(SERVEUR %d) Requete LOGIN reçue de %d : --%s--%s--%s--\n",getpid(),m.expediteur,m.data1,m.data2,m.texte);
                      sem(1,-1);
                      loginn(m);
                      sem(1,1);
                      break; 

      case LOGOUT :  
                      fprintf(stderr,"(SERVEUR %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                      sem(1,-1);
                      decon(m);
                      sem(1,1);
                      break;

      case ACCEPT_USER :
                      fprintf(stderr,"(SERVEUR %d) Requete ACCEPT_USER reçue de %d\n",getpid(),m.expediteur);
                      sem(1,-1);
                      accept(m);
                      sem(1,1);
                      break;

      case REFUSE_USER :
                      fprintf(stderr,"(SERVEUR %d) Requete REFUSE_USER reçue de %d\n",getpid(),m.expediteur);  
                      sem(1,-1);

                      refus(m);

                      sem(1,1);
                      break;

      case SEND :  
                      fprintf(stderr,"(SERVEUR %d) Requete SEND reçue de %d\n",getpid(),m.expediteur);                      
                      sem(1,-1);

                      sen(m);

                      sem(1,1);
                      break; 

      case UPDATE_PUB :
                      fprintf(stderr,"(SERVEUR %d) Requete UPDATE_PUB reçue de %d\n",getpid(),m.expediteur);
                      sem(1,-1);
                      pube();


                      sem(1,1);
                      break;

      case CONSULT :
                      fprintf(stderr,"(SERVEUR %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      if((id1=fork())!=-1){
                        if(id1==0){
                          if(execl("./Consultation","Consultation",NULL)==-1){
                            fprintf(stderr,"(SERVEUR)Erreur de execl");
                          }
                        }
                      }
                      m.type=id1;
                      if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
                        fprintf(stderr,"(SERVEUR %d) erreur de msgsnd",getpid());
                       }
                      break;

      case MODIF1 :
                      fprintf(stderr,"(SERVEUR %d) Requete MODIF1 reçue de %d\n",getpid(),m.expediteur);
                      if((idm=fork())!=-1){
                        if(idm==0){
                          if(execl("./Modification","Modification",NULL)==-1){
                            fprintf(stderr,"(SERVEUR)Erreur de execl");
                          }
                        }
                      }
                      sem(1,-1);
                      ajoutmodif(&m,idm);
                      sem(1,1);
                      m.type=idm;
                      if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
                        fprintf(stderr,"(SERVEUR %d) erreur de msgsnd",getpid());
                      }
                      break;

      case MODIF2 :
                      fprintf(stderr,"(SERVEUR %d) Requete MODIF2 reçue de %d\n",getpid(),m.expediteur);
                      //printf("modif2\n");
                      sem(1,-1);
                      i=0;
                      tr=0;
                      while(tr==0 && i<6){
                        if(tab->connexions[i].pidFenetre==m.expediteur){
                          tr=1;
                          //strcpy(m->data1,tab->connexions[i].nom);
                          c=tab->connexions[i].pidModification;
                        }
                        i++;
                      }
                      //printf("%d\n",c);
                      sem(1,1);
                      m.type=c;
                      m.requete=MODIF2;
                      if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
                        fprintf(stderr,"(SERVEUR %d) erreur de msgsnd",getpid());
                      }

                      break;

      case LOGIN_ADMIN :
                      sem(1,-1);

                      fprintf(stderr,"(SERVEUR %d) Requete LOGIN_ADMIN reçue de %d\n",getpid(),m.expediteur);
                      if(tab->pidAdmin==0 || kill(tab->pidAdmin,0)==0){
                        tab->pidAdmin=m.expediteur;
                        strcpy(m.data1,"OK");  
                      }
                      else{
                        strcpy(m.data1,"KO");
                      }
                      m.type=m.expediteur;
                      if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
                        fprintf(stderr,"(SERVEUR %d) erreur de msgsnd",getpid());
                      }

                      sem(1,1);
                      break;

      case LOGOUT_ADMIN :

                      sem(1,-1);

                      fprintf(stderr,"(SERVEUR %d) Requete LOGOUT_ADMIN reçue de %d\n",getpid(),m.expediteur);
                      tab->pidAdmin=0;


                      sem(1,1);
                      break;

      case NEW_USER :
                      fprintf(stderr,"(SERVEUR %d) Requete NEW_USER reçue de %d : --%s--%s--\n",getpid(),m.expediteur,m.data1,m.data2);
                      if(estPresent(m.data1)>0){
                        strcpy(m.data1,"KO");
                        strcpy(m.texte,"utilisateur deja existant");
                      }
                      else{
                        if(estPresent(m.data1)<=0){
                          ajouteUtilisateur(m.data1,m.data2);
                          strcpy(m.texte,"nouvel utilisateur créé");
                          strcpy(m.data1,"OK");
                          addbd(m);
                        }
                        else{
                          strcpy(m.data1,"KO");
                          strcpy(m.texte,"erreur");
                        }
                      }
                      m.type=m.expediteur;
                    if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
                      printf("(SERVEUR)erreur de msgget\n");
                    }
                      break;

      case DELETE_USER :
                    int i;
                      fprintf(stderr,"(SERVEUR %d) Requete DELETE_USER reçue de %d : --%s--\n",getpid(),m.expediteur,m.data1);
                      i=estPresent(m.data1);
                      if(i>0){
                        UTILISATEUR util;
                        int fp;
                        if((fp=open(FICHIER_UTILISATEURS,O_WRONLY|O_CREAT,0644))==-1){
                          perror("1.Erreur de open()");
                          strcpy(m.data1,"KO");
                          strcpy(m.texte,"ouverture");
                        }
                        else{
                          fprintf(stderr,"(SERVEUR %d) Prise bloquante du sémaphore 0\n",getpid());
                          sem(0,-1);
                          sleep(1);

                          // Connexion à la base de donnée
                          MYSQL *connexion = mysql_init(NULL);
                          fprintf(stderr,"(SERVEUR %d) Connexion à la BD\n",getpid());
                          if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
                          {
                            fprintf(stderr,"(SERVEUR) Erreur de connexion à la base de données...\n");
                            exit(1);  
                          }

                          // Recherche des infos dans la base de données
                          fprintf(stderr,"(SERVEUR %d) Consultation en BD (%s)\n",getpid(),m.data1);
                          MYSQL_RES  *resultat;
                          MYSQL_ROW  tuple;
                          char requete[200];
                          sprintf(requete," delete from UNIX_FINAL WHERE nom='%s'",m.data1);
                          mysql_query(connexion,requete),
                          printf("%d \n",i);
                          strcpy(util.nom,"-1");
                          util.hash=-1;
                          printf("%d \n",lseek(fp,(i-1)*sizeof(util),SEEK_SET));
                          write(fp,&util,sizeof(util));
                          close(fp);

                           mysql_close(connexion);

                          // Libération du semaphore 0
                          sem(0,1);
                          fprintf(stderr,"(SERVEUR %d) Libération du sémaphore 0\n",getpid());
                          strcpy(m.data1,"OK");
                          strcpy(m.texte,"suppression reussi");
                       }
                      }
                      else{
                        strcpy(m.data1,"KO");
                        strcpy(m.texte,"utilisateur inconnu....");
                      }
                      m.type=m.expediteur;
                      if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
                        printf("(SERVEUR)erreur de msgget\n");
                      }
                      break;

      case NEW_PUB :
                      fprintf(stderr,"(SERVEUR %d) Requete NEW_PUB reçue de %d\n",getpid(),m.expediteur);
                      PUBLICITE pub;
                      int fp;
                      strcpy(pub.texte,m.texte);
                      pub.nbSecondes=atoi(m.data1);
                      //printf("%s,%d \n",pub.texte,pub.nbSecondes);
                      if((fp=open("publicites.dat",O_WRONLY|O_APPEND,0644))==-1){
                        if((fp=open("publicites.dat",O_WRONLY|O_CREAT|O_APPEND,0644))==-1){
                          perror("(SERVEUR)1.Erreur de open()");
                        }
                        else{
                          kill(tab->pidPublicite,SIGUSR1);
                          write(fp,&pub,sizeof(pub));
                          close(fp);
                        }
                      }
                      else{
                        write(fp,&pub,sizeof(pub));
                        close(fp);
                      }
                      break;
    }
    afficheTab();
  }
}

void afficheTab()
{
  sem(1,-1);
  fprintf(stderr,"Pid Serveur 1 : %d\n",tab->pidServeur1);
  fprintf(stderr,"Pid Serveur 2 : %d\n",tab->pidServeur2);
  fprintf(stderr,"Pid Publicite : %d\n",tab->pidPublicite);
  fprintf(stderr,"Pid Admin     : %d\n",tab->pidAdmin);
  for (int i=0 ; i<6 ; i++)
    fprintf(stderr,"%6d -%20s- %6d %6d %6d %6d %6d - %6d\n",tab->connexions[i].pidFenetre,
                                                      tab->connexions[i].nom,
                                                      tab->connexions[i].autres[0],
                                                      tab->connexions[i].autres[1],
                                                      tab->connexions[i].autres[2],
                                                      tab->connexions[i].autres[3],
                                                      tab->connexions[i].autres[4],
                                                      tab->connexions[i].pidModification);
  fprintf(stderr,"\n");
  sem(1,1);
}
void connect(int a,int b){
  int tr=0,i=0;
  while(tr==0 && i<6){
    if(tab->connexions[i].pidFenetre==b){
      tr=1;
      tab->connexions[i].pidFenetre=a;
    }
    i++;
  }
  if(tr==0){
    if(b==0){
      kill(a,SIGINT);
    }
  }
}
void logi(int pid,const char*nom){
  int tr=0,i=0;
  while(tr==0 && i<6){
    if(tab->connexions[i].pidFenetre==pid){
      tr=1;
      strcpy(tab->connexions[i].nom,nom);
    }
    i++;
  }
}
int verificonect(const char*nom){
  int tr=0,i=0;
  while(tr==0 && i<6){
    if(strcmp(tab->connexions[i].nom,nom)==0){
      tr=1;
    }
    i++;
  }
  return tr;
}

void loginn(MESSAGE m){
  int i;
  char chaine[30];
  MESSAGE u;
  strcpy(chaine,m.data2);
  if(verificonect(m.data2)==0){
    if(strcmp(m.data1,"1")==0){
      if(estPresent(m.data2)>0){
        strcpy(m.data1,"KO");
        strcpy(m.texte,"utilisateur deja existant");
      }
      else{
        if(estPresent(m.data2)<=0){
          strcpy(m.data1,"OK");
          ajouteUtilisateur(m.data2,m.texte);
          logi(m.expediteur,m.data2);
          strcpy(m.texte,"nouvel utilisateur créé");
          addbd(m);
        }
        else{
          strcpy(m.data1,"KO");
          strcpy(m.texte,"erreur");
        }
      }
    }
    else{
      i=estPresent(m.data2);
      if(i>0){
        if(verifieMotDePasse(i,m.texte)==1){
          strcpy(m.data1,"OK");
          logi(m.expediteur,m.data2);
          strcpy(m.texte,"RE-bonjour cher utilisateur");
        }
        else{
          strcpy(m.data1,"KO");
          strcpy(m.texte,"mot de passe incorrect....");
        }
      }
      else{
        strcpy(m.texte,"utilisateur inconnu....");
      }
    }
  }
  else{
    strcpy(m.data1,"KO");
    strcpy(m.texte,"utilisateur deja connecter ailleurs....");
  }

  m.type=m.expediteur;
  if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
    printf("(SERVEUR)erreur de msgget\n");
  }

  kill(m.expediteur,SIGUSR1);
  if(strcmp(m.data1,"OK")==0){
    m.requete=ADD_USER;
    for(i=0;i<6;i++){
      printf("%d,%d,%d \n",i,strcmp(tab->connexions[i].nom,chaine),tab->connexions[i].pidFenetre);
      if(strcmp(tab->connexions[i].nom,chaine)!=0 && strcmp(tab->connexions[i].nom,"")!=0){
        sleep(1);
        m.type=m.expediteur;
        strcpy(m.data1,tab->connexions[i].nom);
        if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
          printf("(SERVEUR) erreur de msgget");
        }
        m.type=tab->connexions[i].pidFenetre;
        strcpy(m.data1,chaine);
        if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
          printf("(SERVEUR) erreur de msgget");
        }
        kill(m.type,SIGUSR1);
        kill(m.expediteur,SIGUSR1);
      }
    }
  }
}

void HandlerSIGINT(int sig){

  sem(1,-1);

  if(tab->pidServeur1==getpid()){
    tab->pidServeur1=0;
  }
  else{
    if(tab->pidServeur2==getpid()){
      tab->pidServeur2=0;
    }
  }
  if((tab->pidServeur1==0 || kill(tab->pidServeur1,0)==-1)&&(tab->pidServeur2==0 || kill(tab->pidServeur2,0)==-1)){
    int i=0,tr=0,idm;
    while(tr==0 && i<6){
      if(tab->connexions[i].pidFenetre!=0){
        kill(tab->connexions[i].pidFenetre,SIGQUIT);
      }
      if((idm=tab->connexions[i].pidModification)!=0){
        kill(idm,SIGQUIT);
        //strcpy(m->data1,tab->connexions[i].nom);
      } 
      i++;
    }
    if(tab->pidAdmin!=0){
      kill(tab->pidAdmin,SIGQUIT);
    }
    if(id1!=0){
      kill(id1,SIGQUIT);
    }
    //kill(tab->pidPublicite,SIGQUIT);
    if (shmdt(tab) == -1) {
      perror("(SERVEUR)Erreur de shmdt");
    }

    msgctl(idQ,IPC_RMID,NULL);
    mysql_close(connexion);
    if (shmctl(idShm,IPC_RMID,NULL) == -1)
    {
      printf("%d\n",idShm);
      perror("(SERVEUR)Erreur de shmctl");
      exit(1);
    }
    if (shmctl(idShms,IPC_RMID,NULL) == -1)
    {
      perror("(SERVEUR)Erreur de shmctl");
      exit(1);
    }
    if (semctl(idSem,0,IPC_RMID) == -1)
    {
      perror("(SERVEUR)Erreur de semctl (3)");
      exit(1);
    }
  }
  exit(0);

}

void HandlerSIGCHILD(int sig){
  int id, status;
  if((id=wait(&status))!=-1){
    if(id==tab->pidAdmin){
      tab->pidAdmin=0;
    }
    else{
      if(id==id1){
        id1=0;
      }
      else{
        int tr=0,i=0;
        while(tr==0 && i<6){
          if(tab->connexions[i].pidModification==id){
            tr=1;
            tab->connexions[i].pidModification=0;
          }
          i++;
        }
      }
    }
  } 
  siglongjmp(contexte,1);
}

void accept(MESSAGE m){
  int i=0,j=0;
  int tr=0;
    while(tr==0 && i<6){
      if(tab->connexions[i].pidFenetre==m.expediteur){
        tr=1;
      }
      i++;
    }
    tr=0;
    while(tr==0 && j<6){
      if(strcmp(tab->connexions[j].nom,m.data1)==0){
        tr=1;
      }
      j++;
    }
    tr=0;
    while(tab->connexions[i-1].autres[tr]!=0){
      tr++;
    }
    tab->connexions[i-1].autres[tr]=tab->connexions[j-1].pidFenetre;

}
void refus(MESSAGE m){
  int i=0,j=0;
  int tr=0;
    while(tr==0 && i<6){
      if(tab->connexions[i].pidFenetre==m.expediteur){
        tr=1;
      }
      i++;
    }
    tr=0;
    while(tr==0 && j<6){
      if(strcmp(tab->connexions[j].nom,m.data1)==0){
        tr=1;
      }
      j++;
    }
    tr=0;
    while(tab->connexions[i-1].autres[tr]!=tab->connexions[j-1].pidFenetre){
      tr++;
    }
    tab->connexions[i-1].autres[tr]=0;
}
void decon(MESSAGE m){
  MESSAGE u;
  char chaine[30];
  int i=0,j,tr=0;
  while(tr==0 && i<6){
    if(tab->connexions[i].pidFenetre==m.expediteur){
      tr=1;
      strcpy(chaine,tab->connexions[i].nom);
      strcpy(tab->connexions[i].nom,"");
      for(j=0;j<5;j++){
        tab->connexions[i].autres[j]=0;
      }
    }
    i++;
  }
  tr=i-1;
  for(i=0;i<6;i++){
    if(tab->connexions[i].pidFenetre!=m.expediteur && tab->connexions[i].pidFenetre!=0 && strcmp(tab->connexions[i].nom,"")!=0){
      for(j=0;j<5;j++){
        if(tab->connexions[i].autres[j]==m.expediteur){
          tab->connexions[i].autres[j]=0;
        }
        u.type=tab->connexions[i].pidFenetre;
        u.requete=REMOVE_USER;
      }
      strcpy(u.data1,chaine);
      if(msgsnd(idQ,&u,sizeof(MESSAGE)-sizeof(long),0)==-1){
        printf("(SERVEUR)erreur de msgget\n");
      }
      kill(u.type,SIGUSR1);
      sleep(1);
    }
  }
}
void sen(MESSAGE m){
  MESSAGE u;
  u.requete=SEND;
  char chaine[30];
  int i,j,tr=0;
  i=0;
  while(tr==0 && i<6){
    if(tab->connexions[i].pidFenetre==m.expediteur){
      tr=1;
      strcpy(chaine,tab->connexions[i].nom);
    }
    i++;
  }
  i=0;
  tr=0;
  while(tr==0 && i<6){
    if(tab->connexions[i].pidFenetre==m.expediteur){
      for(j=0;j<5;j++){
        if(tab->connexions[i].autres[j]!=0){
          u.type=tab->connexions[i].autres[j];
          strcpy(u.data1,chaine);
          strcpy(u.texte,m.data1);
          msgsnd(idQ,&u,sizeof(MESSAGE)-sizeof(long),0);
          kill(u.type,SIGUSR1);
        }
      }
    }
    i++;
  }
}
void pube(){
  int i=0;
  for(i=0;i<6;i++){
    if(tab->connexions[i].pidFenetre!=0){
      //printf("tue la fenetre %d",tab->connexions[i].pidFenetre);
      kill(tab->connexions[i].pidFenetre,SIGUSR2);
    }
  }
}
void addbd(MESSAGE m){
  char requete[256];
  //printf("ajoutons \n");
  sem(0,-1);
  MYSQL* connexion = mysql_init(NULL);
  mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0);

  sprintf(requete,"insert into UNIX_FINAL values (NULL,'%s','%s','%s');",m.data2,"-----","-----");
  mysql_query(connexion,requete);
  mysql_close(connexion);
  sem(0,1);
}
void ajoutmodif(MESSAGE*m,int idm){
  int tr=0,i=0;
  while(tr==0 && i<6){
    if(tab->connexions[i].pidFenetre==m->expediteur){
      tr=1;
      strcpy(m->data1,tab->connexions[i].nom);
      tab->connexions[i].pidModification=idm;
    }
    i++;
  }
}

void sem(int a,int b){
  action.sem_num=a;
  action.sem_op=b;
  action.sem_flg=0;
  semop(idSem,&action,1);
}