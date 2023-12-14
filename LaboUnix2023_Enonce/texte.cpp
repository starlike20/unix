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

int main(int argc, char const *argv[])
{
  MYSQL *connexion = mysql_init(NULL);
  fprintf(stderr,"(CONSULTATION %d) Connexion à la BD\n",getpid());
  if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(CONSULTATION) Erreur de connexion à la base de données...\n");
    exit(1);  
  }

  // Recherche des infos dans la base de données
  fprintf(stderr,"(CONSULTATION %d) Consultation en BD (%s)\n",getpid(),argv[1]);
  MYSQL_RES  *resultat;
  MYSQL_ROW  tuple;
  char requete[200];
  sprintf(requete," select * from UNIX_FINAL WHERE nom='%s'",argv[1]);
  mysql_query(connexion,requete),
  resultat = mysql_store_result(connexion);
  if ((tuple = mysql_fetch_row(resultat)) != NULL) {
   printf("%s \n",tuple[0]);
   printf("%s \n",tuple[1]);
   printf("%s \n",tuple[2]);
  }
  return 0;
}