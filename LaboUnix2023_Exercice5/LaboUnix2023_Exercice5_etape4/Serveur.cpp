#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include <string.h>
#include "protocole.h" // contient la cle et la structure d'un message

int idQ;
int pid1,pid2;

void HandlerSIGINT(int sig);

int main()
{

  MESSAGE requete;
  pid_t destinataire;

  // Armement du signal SIGINT
  // TO DO (etape 6)
  struct sigaction B;
  B.sa_handler = HandlerSIGINT;
  B.sa_flags = 0;
  sigemptyset(&B.sa_mask);
  sigaction(SIGINT,&B,NULL);

  // Creation de la file de message
  fprintf(stderr,"(SERVEUR) Creation de la file de messages\n");
  // TO DO (etape 2)
  if ((idQ = msgget(CLE,IPC_CREAT | 0600)) == -1)
  {
    perror("Erreur de msgget");
    exit(1);
  }

  // Attente de connection de 2 clients
  fprintf(stderr,"(SERVEUR) Attente de connection d'un premier client...\n");
  // TO DO (etape 5)
  msgrcv(idQ,&requete,sizeof(MESSAGE)-sizeof(long),1,0);
  pid1=requete.expediteur;
  fprintf(stderr,"(SERVEUR) Attente de connection d'un second client...\n");
  // TO DO (etape 5)
  msgrcv(idQ,&requete,sizeof(MESSAGE)-sizeof(long),1,0);
  pid2=requete.expediteur;

  while(1) 
  {
    char m[80];
    strcpy(m,"(SERVEUR)");
    // TO DO (etapes 3, 4 et 5)
  	fprintf(stderr,"(SERVEUR) Attente d'une requete...\n");
    msgrcv(idQ,&requete,sizeof(MESSAGE)-sizeof(long),1,0);
    fprintf(stderr,"(SERVEUR) Requete recue de %d : --%s--\n",requete.expediteur,requete.texte);
    destinataire=requete.expediteur;
    requete.expediteur=1;
    strcpy(requete.texte,strcat(m,requete.texte));
    if(destinataire==pid1){
      requete.type=pid2;
      msgsnd(idQ,&requete,sizeof(MESSAGE)-sizeof(long),0);
      kill(pid2,SIGUSR1);
      fprintf(stderr,"(SERVEUR) Envoi de la reponse a %d\n",pid2);
    }
    else{
      requete.type=pid1;
      msgsnd(idQ,&requete,sizeof(MESSAGE)-sizeof(long),0);
      kill(pid1,SIGUSR1);
      fprintf(stderr,"(SERVEUR) Envoi de la reponse a %d\n",pid1);
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Handlers de signaux ////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TO DO (etape 6)
void HandlerSIGINT(int sig){
  msgctl(idQ,IPC_RMID,NULL);
  exit(0);
}