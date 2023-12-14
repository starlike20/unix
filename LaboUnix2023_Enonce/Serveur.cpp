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
int idQ,idShm,idSem;
TAB_CONNEXIONS *tab;

void afficheTab();
void connect(int a,int b);
void logi(int pid,const char*nom);
void loginn(MESSAGE m);
int verificonect(const char*nom);
void HandlerSIGINT(int sig);
void accept(MESSAGE m);
void decon(MESSAGE m);
void refus(MESSAGE m);
void sen(MESSAGE m);
void pube();
void addbd(MESSAGE m);
void ajoutmodif(MESSAGE*m,int idm);

MYSQL* connexion;


int main()
{
  int id1,idm;
  // Connection à la BD
  connexion = mysql_init(NULL);
  if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(SERVEUR) Erreur de connexion à la base de données...\n");
    exit(1);  
  }

  // Armement des signaux
  struct sigaction B;
  B.sa_handler = HandlerSIGINT;
  B.sa_flags = 0;
  sigemptyset(&B.sa_mask);
  sigaction(SIGINT,&B,NULL);

  // Creation des ressources
  fprintf(stderr,"(SERVEUR %d) Creation de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,IPC_CREAT | 0600)) == -1)  // CLE definie dans protocole.h
  {
    perror("(SERVEUR) Erreur de msgget");
    exit(1);
  }

  if ((idShm = shmget(CLE,200,IPC_CREAT | IPC_EXCL | 0600)) == -1){
    perror("(SERVEUR) Erreur de shmget");
    exit(1);
  }
  if ((idSem = semget(CLE,1,IPC_CREAT | IPC_EXCL | 0600)) == -1)
  {
    perror("Erreur de semget");
    exit(1);
  }
  struct sembuf action;
  action.sem_num=0;
  action.sem_op=1;
  action.sem_flg=0;
  semop(idSem,&action,1);

  // Initialisation du tableau de connexions
  fprintf(stderr,"(SERVEUR %d) Initialisation de la table des connexions\n",getpid());
  tab = (TAB_CONNEXIONS*) malloc(sizeof(TAB_CONNEXIONS)); 

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

  afficheTab();

  // Creation du processus Publicite
  /*int idp,id;
  if((id=fork())!=-1){
    if(id==0){
      if(execl("./Publicite","Publicite",NULL)==-1){
        fprintf(stderr,"(SERVEUR)Erreur de execl");
      }
    }
  }
  tab->pidPublicite=id;*/

  int i,k,j;
  MESSAGE m;
  MESSAGE reponse;
  char requete[200];
  MYSQL_RES  *resultat;
  MYSQL_ROW  tuple;
  PUBLICITE pub;

  while(1)
  {
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
                      connect(m.expediteur,0);
                      break; 

      case DECONNECT :  
                      fprintf(stderr,"(SERVEUR %d) Requete DECONNECT reçue de %d\n",getpid(),m.expediteur);
                      logi(m.expediteur,"");
                      connect(0,m.expediteur);
                      break; 

      case LOGIN :  
                      fprintf(stderr,"(SERVEUR %d) Requete LOGIN reçue de %d : --%s--%s--%s--\n",getpid(),m.expediteur,m.data1,m.data2,m.texte);
                      loginn(m);
                      break; 

      case LOGOUT :  
                      fprintf(stderr,"(SERVEUR %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                      decon(m);
                      break;

      case ACCEPT_USER :
                      fprintf(stderr,"(SERVEUR %d) Requete ACCEPT_USER reçue de %d\n",getpid(),m.expediteur);
                      accept(m);
                      break;

      case REFUSE_USER :
                      fprintf(stderr,"(SERVEUR %d) Requete REFUSE_USER reçue de %d\n",getpid(),m.expediteur);
                      refus(m);
                      break;

      case SEND :  
                      fprintf(stderr,"(SERVEUR %d) Requete SEND reçue de %d\n",getpid(),m.expediteur);
                      sen(m);
                      break; 

      case UPDATE_PUB :
                      fprintf(stderr,"(SERVEUR %d) Requete UPDATE_PUB reçue de %d\n",getpid(),m.expediteur);
                      pube();
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
                      ajoutmodif(&m,idm);
                      m.type=idm;
                      if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
                        fprintf(stderr,"(SERVEUR %d) erreur de msgsnd",getpid());
                      }
                      break;

      case MODIF2 :
                      fprintf(stderr,"(SERVEUR %d) Requete MODIF2 reçue de %d\n",getpid(),m.expediteur);
                      m.type=idm;
                      m.requete=MODIF2;
                      if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
                        fprintf(stderr,"(SERVEUR %d) erreur de msgsnd",getpid());
                      }

                      break;

      case LOGIN_ADMIN :
                      fprintf(stderr,"(SERVEUR %d) Requete LOGIN_ADMIN reçue de %d\n",getpid(),m.expediteur);
                      if(tab->pidAdmin==0){
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
                      break;

      case LOGOUT_ADMIN :
                      fprintf(stderr,"(SERVEUR %d) Requete LOGOUT_ADMIN reçue de %d\n",getpid(),m.expediteur);
                      tab->pidAdmin=0;
                      break;

      case NEW_USER :
                      fprintf(stderr,"(SERVEUR %d) Requete NEW_USER reçue de %d : --%s--%s--\n",getpid(),m.expediteur,m.data1,m.data2);
                      loginn(m);
                      break;

      case DELETE_USER :
                      fprintf(stderr,"(SERVEUR %d) Requete DELETE_USER reçue de %d : --%s--\n",getpid(),m.expediteur,m.data1);
                      break;

      case NEW_PUB :
                      fprintf(stderr,"(SERVEUR %d) Requete NEW_PUB reçue de %d\n",getpid(),m.expediteur);
                      break;
    }
    afficheTab();
  }
}

void afficheTab()
{
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
  msgctl(idQ,IPC_RMID,NULL);
  delete[] tab;
  mysql_close(connexion);
  if (shmctl(idShm,IPC_RMID,NULL) == -1)
  {
    perror("(SERVEUR)Erreur de shmctl");
    exit(1);
  }
  if (semctl(idSem,0,IPC_RMID) == -1)
  {
    perror("Erreur de semctl (3)");
    exit(1);
  }
  kill(tab->pidPublicite,SIGQUIT);
  exit(0);
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
      printf("tue la fenetre %d",tab->connexions[i].pidFenetre);
      kill(tab->connexions[i].pidFenetre,SIGUSR2);
    }
  }
}
void addbd(MESSAGE m){
  char requete[256];
  printf("ajoutons \n");
  MYSQL* connexion = mysql_init(NULL);
  mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0);

  sprintf(requete,"insert into UNIX_FINAL values (NULL,'%s','%s','%s');",m.data2,"-----","-----");
  mysql_query(connexion,requete);
  mysql_close(connexion);
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

