#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "protocole.h" // contient la cle et la structure d'un message

int idQ, idShm;
int fd;
char*pShm;

int main()
{
  int fd;

  // Armement des signaux

  // Masquage de SIGINT
  sigset_t mask;
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);


  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(PUBLICITE %d) Recuperation de l'id de la file de messages\n",getpid());
  if((idQ = msgget(CLE,0))==-1){
    fprintf(stderr,"(PUBLICITE %d) erreur de Recuperation de l'id de la file de messages\n",getpid());
  }


  // Recuperation de l'identifiant de la mémoire partagée
  fprintf(stderr,"(PUBLICITE %d) Recuperation de l'id de la mémoire partagée\n",getpid());
  if ((idShm = shmget(CLE,0,0)) == -1)  
  {
    fprintf(stderr,"(PUBLICITE %d) Recuperation de l'id de la mémoire partagée\n",getpid());
    exit(1);
  }


  // Attachement à la mémoire partagée
  if((pShm = (char*)shmat(idShm,NULL,0))==NULL)
  {
    fprintf(stderr,"(PUBLICITE %d) erreur d'attachement",getpid());
  }


  // Ouverture du fichier de publicité
  if ((fd = open("publicites.dat",O_RDONLY, 0644)) == -1)
  {
    fprintf(stderr,"(PUBLICITE %d) erreur d'ouverture du fichier publicites.dat",getpid());
    exit(1);
  }


  while(1)
  {
    MESSAGE m;
  	PUBLICITE pub;

    // Lecture d'une publicité dans le fichier

    if(read(fd,&pub,sizeof(PUBLICITE))<sizeof(PUBLICITE)){
      lseek(fd,0,SEEK_SET);
    }
    else{

      // Ecriture en mémoire partagée

      strcpy(pShm,pub.texte);
      m.type=1;
      m.requete=UPDATE_PUB;

      // Envoi d'une requete UPDATE_PUB au serveur

      if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
        fprintf(stderr,"(PUBLICITE %d) erreur msgsnd",getpid());
      }
      sleep(pub.nbSecondes);
    }
  }
}

