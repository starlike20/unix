#include "windowclient.h"
#include "ui_windowclient.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

extern WindowClient *w;

#include "protocole.h" // contient la cle et la structure d'un message

extern char nomClient[40];
int idQ; // identifiant de la file de message

void HandlerSIGUSR1(int sig);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
WindowClient::WindowClient(QWidget *parent):QMainWindow(parent),ui(new Ui::WindowClient)
{
  ui->setupUi(this);
  setWindowTitle(nomClient);

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(CLIENT %s %d) Recuperation de l'id de la file de messages\n",nomClient,getpid());
  // TO DO (etape 2)
  if ((idQ = msgget(CLE,IPC_CREAT | 0600)) == -1){
    perror("Erreur de msgget");
  }

  // Envoi d'une requete d'identification
  // TO DO (etape 5)
  MESSAGE m;
  m.type=1;
  m.expediteur=getpid();
  strcpy(m.texte,"new");
  msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0);

  // Armement du signal SIGUSR1
  // TO DO (etape 4)
  struct sigaction B;
  B.sa_handler = HandlerSIGUSR1;
  B.sa_flags = 0;
  sigemptyset(&B.sa_mask);
  sigaction(SIGUSR1,&B,NULL);
}

WindowClient::~WindowClient()
{
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setRecu(const char* Text)
{
  //fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditRecu->clear();
    return;
  }
  ui->lineEditRecu->setText(Text);
}

void WindowClient::setAEnvoyer(const char* Text)
{
  //fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditEnvoyer->clear();
    return;
  }
  ui->lineEditEnvoyer->setText(Text);
}

const char* WindowClient::getAEnvoyer()
{
  if (ui->lineEditEnvoyer->text().size())
  { 
    strcpy(aEnvoyer,ui->lineEditEnvoyer->text().toStdString().c_str());
    return aEnvoyer;
  }
  return NULL;
}

const char* WindowClient::getRecu()
{
  if (ui->lineEditRecu->text().size())
  { 
    strcpy(recu,ui->lineEditRecu->text().toStdString().c_str());
    return recu;
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonEnvoyer_clicked()
{
  MESSAGE m;
  fprintf(stderr,"Clic sur le bouton Envoyer\n");
  // TO DO (etapes 2, 3, 4)
  m.type=1;
  m.expediteur=getpid();
  strcpy(m.texte,getAEnvoyer());
  if (msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0) == -1)
  {
    perror("Erreur de msgsnd");
    exit(1);
  }
}

void WindowClient::on_pushButtonQuitter_clicked()
{
  fprintf(stderr,"Clic sur le bouton Quitter\n");
  exit(1);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Handlers de signaux ////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
// TO DO (etape 4)
void HandlerSIGUSR1(int sig){
  printf("sig\n");
  MESSAGE m;
  if(msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0)==-1){
    perror("Erreur de msgrcv");
    exit(1);
  }
  w->setRecu(m.texte);
}
