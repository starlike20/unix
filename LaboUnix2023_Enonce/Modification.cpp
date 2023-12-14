#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <mysql.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "protocole.h"
#include "FichierUtilisateur.h"

int idQ,idSem;

int main()
{
  char nom[40];

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(MODIFICATION %d) Recuperation de l'id de la file de messages\n",getpid());
  if((idQ = msgget(CLE,0))==-1)
  {
    fprintf(stderr,"(CONSULTATION %d) erreur de Recuperation de l'id de la file de messages\n",getpid());
  }
  // Recuperation de l'identifiant du sémaphore
  if ((idSem = semget(CLE,0,0)) == -1)
  {
    fprintf(stderr,"(CONSULTATION %d)Erreur de semget",getpid());
    exit(1);
  }

  MESSAGE m;
  if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
  {
    perror("(MODIFICATION) Erreur de msgrcv");
  }
  // Lecture de la requête MODIF1
  fprintf(stderr,"(MODIFICATION %d) Lecture requete MODIF1\n",getpid());
  // Tentative de prise non bloquante du semaphore 0 (au cas où un autre utilisateut est déjà en train de modifier)
  struct sembuf action;
  action.sem_num=0;
  action.sem_op=-1;
  action.sem_flg=IPC_NOWAIT;
  if(semop(idSem,&action,1)==-1){
    m.type=m.expediteur;
    strcpy(m.data1,"KO");
    strcpy(m.data2,"KO");
    strcpy(m.texte,"KO");
    perror("Modification semop occuper");
    if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
      fprintf(stderr,"(MODIFICATION %d) erreur de msgsnd",getpid());
    }
  }

  // Connexion à la base de donnée
  MYSQL *connexion = mysql_init(NULL);
  fprintf(stderr,"(MODIFICATION %d) Connexion à la BD\n",getpid());
  if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(MODIFICATION) Erreur de connexion à la base de données...\n");
    exit(1);  
  }

  // Recherche des infos dans la base de données
  fprintf(stderr,"(MODIFICATION %d) Consultation en BD pour --%s--\n",getpid(),m.data1);
  strcpy(nom,m.data1);
  MYSQL_RES  *resultat;
  MYSQL_ROW  tuple;
  char requete[200];
   strcpy(nom,m.data1);
  sprintf(requete," select * from UNIX_FINAL WHERE nom='%s'",m.data1);
  mysql_query(connexion,requete),
  resultat = mysql_store_result(connexion);
  if ((tuple = mysql_fetch_row(resultat)) != NULL) {
    strcpy(m.data1," ");
    strcpy(m.data2,tuple[2]);
    strcpy(m.texte,tuple[3]);
  }

  // Construction et envoi de la reponse
  fprintf(stderr,"(MODIFICATION %d) Envoi de la reponse\n",getpid());
  m.type=m.expediteur;
  if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
      fprintf(stderr,"(MODIFICATION %d) erreur de msgsnd",getpid());
  }

  // Attente de la requête MODIF2
  fprintf(stderr,"(MODIFICATION %d) Attente requete MODIF2...\n",getpid());
  if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
  {
    perror("(MODIFICATION) Erreur de msgrcv");
  }

  // Mise à jour base de données
  fprintf(stderr,"(MODIFICATION %d) Modification en base de données pour --%s--\n",getpid(),nom);
  sprintf(requete,"UPDATE UNIX_FINAL SET gsm = '%s', email = '%s' WHERE nom = '%s'",m.data2,m.texte,nom);
  mysql_query(connexion,requete);
  //sprintf(requete,...);
  //mysql_query(connexion,requete);

  // Mise à jour du fichier si nouveau mot de passe
  if(strcmp(m.data1,"")!=0){
    int fp ,i=0;
    UTILISATEUR util;
    if((fp=open(FICHIER_UTILISATEURS, O_RDWR|O_CREAT))==-1){
      perror("Erreur de open()");
      return -1;
    }
    else{
      while(read(fp,&util,sizeof(util))>0 && i==0){
        if(strcmp(nom,util.nom)==0){
          i=1;
          printf("%s \n",m.data1);
          util.hash=hash(m.data1);
          lseek(fp,-sizeof(util),SEEK_CUR);
          write(fp,&util,sizeof(util));
        }
      }
    }
  }


  // Deconnexion BD
  mysql_close(connexion);

  // Libération du semaphore 0
  fprintf(stderr,"(MODIFICATION %d) Libération du sémaphore 0\n",getpid());
  action.sem_num=0;
  action.sem_op=1;
  action.sem_flg=IPC_NOWAIT;
  semop(idSem,&action,1);
  exit(0);
}