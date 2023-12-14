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
#include <errno.h>

int idQ,idSem;

int main()
{
  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(CONSULTATION %d) Recuperation de l'id de la file de messages\n",getpid());
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
  struct sembuf action;
  // Lecture de la requête CONSULT
  fprintf(stderr,"(CONSULTATION %d) Lecture requete CONSULT\n",getpid());
  if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1){
    fprintf(stderr,"(SERVEUR) Erreur de msgrcv",getpid());
    exit(1);
  }
  // Tentative de prise bloquante du semaphore 0
  fprintf(stderr,"(CONSULTATION %d) Prise bloquante du sémaphore 0\n",getpid());
  action.sem_num=0;
  action.sem_op=-1;
  action.sem_flg=0;
  semop(idSem,&action,1);
  sleep(1);

  // Connexion à la base de donnée
  MYSQL *connexion = mysql_init(NULL);
  fprintf(stderr,"(CONSULTATION %d) Connexion à la BD\n",getpid());
  if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(CONSULTATION) Erreur de connexion à la base de données...\n");
    exit(1);  
  }

  // Recherche des infos dans la base de données
  fprintf(stderr,"(CONSULTATION %d) Consultation en BD (%s)\n",getpid(),m.data1);
  MYSQL_RES  *resultat;
  MYSQL_ROW  tuple;
  char requete[200];
  sprintf(requete," select * from UNIX_FINAL WHERE nom='%s'",m.data1);
  mysql_query(connexion,requete),
  resultat = mysql_store_result(connexion);
  if ((tuple = mysql_fetch_row(resultat)) != NULL) {
    strcpy(m.data1,"OK");
    strcpy(m.data2,tuple[2]);
    strcpy(m.texte,tuple[3]);
  }
  else{
    strcpy(m.data1,"KO");
    strcpy(m.texte,"non trouve");

  }
  // Construction et envoi de la reponse
  m.type=m.expediteur;
  if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
      fprintf(stderr,"(CONSULTATION %d) erreur de msgsnd",getpid());
  }
  kill(m.type,SIGUSR1);

  // Deconnexion BD
  mysql_close(connexion);

  // Libération du semaphore 0
  action.sem_num=0;
  action.sem_op=1;
  action.sem_flg=0;
  semop(idSem,&action,1);
  fprintf(stderr,"(CONSULTATION %d) Libération du sémaphore 0\n",getpid());

  exit(0);
}