#include "windowadmin.h"
#include "ui_windowadmin.h"
#include <QMessageBox>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>

extern int idQ;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
WindowAdmin::WindowAdmin(QWidget *parent):QMainWindow(parent),ui(new Ui::WindowAdmin)
{
  MESSAGE m;
    ui->setupUi(this);
    fprintf(stderr,"(CLIENT %d) Recuperation de l'id de la file de messages\n",getpid());
    if((idQ = msgget(CLE,0))==-1)
    {
      fprintf(stderr,"(CLIENT %d) erreur de Recuperation de l'id de la file de messages\n",getpid());
    }
    m.type=1;
    m.requete=LOGIN_ADMIN;
    m.expediteur=getpid();
    if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
      fprintf(stderr,"(CLIENT %d) erreur de msgsnd",getpid());
    }
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1){
      fprintf(stderr,"(SERVEUR) Erreur de msgrcv %d",getpid());
      exit(1);
    }
    if(strcmp(m.data1,"KO")==0){
      dialogueErreur("Erreur","une fenetre amin deja connecter");
      exit(0);
    }
    ::close(2);
}

WindowAdmin::~WindowAdmin()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowAdmin::setNom(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditNom->clear();
    return;
  }
  ui->lineEditNom->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowAdmin::getNom()
{
  strcpy(nom,ui->lineEditNom->text().toStdString().c_str());
  return nom;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowAdmin::setMotDePasse(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditMotDePasse->clear();
    return;
  }
  ui->lineEditMotDePasse->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowAdmin::getMotDePasse()
{
  strcpy(motDePasse,ui->lineEditMotDePasse->text().toStdString().c_str());
  return motDePasse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowAdmin::setTexte(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditTexte->clear();
    return;
  }
  ui->lineEditTexte->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowAdmin::getTexte()
{
  strcpy(texte,ui->lineEditTexte->text().toStdString().c_str());
  return texte;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowAdmin::setNbSecondes(int n)
{
  char Text[10];
  sprintf(Text,"%d",n);
  ui->lineEditNbSecondes->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowAdmin::getNbSecondes()
{
  char tmp[10];
  strcpy(tmp,ui->lineEditNbSecondes->text().toStdString().c_str());
  return atoi(tmp);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions permettant d'afficher des boites de dialogue /////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowAdmin::dialogueMessage(const char* titre,const char* message)
{
   QMessageBox::information(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowAdmin::dialogueErreur(const char* titre,const char* message)
{
   QMessageBox::critical(this,titre,message);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowAdmin::on_pushButtonAjouterUtilisateur_clicked()
{
  // TO DO
  MESSAGE m;
  m.type=1;
  strcpy(m.data1,getNom());
  strcpy(m.data2,getMotDePasse());
  m.requete=NEW_USER;
  m.expediteur=getpid();
  fprintf(stderr,"(ADMIN %d) Attente d'une requete...\n",getpid());
  if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
    fprintf(stderr,"(ADMIN %d) erreur de msgsnd",getpid());
  }
  fprintf(stderr,"(ADMIN %d) envoi de la requete au serveur\n",getpid());
  if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
  {
    perror("(ADMIN) Erreur de msgrcv");
  }
  if(strcmp(m.data1,"OK")==0){
    dialogueMessage("ajout",m.texte);
  }
  else{
    dialogueErreur("ajout",m.texte);
  }

}

void WindowAdmin::on_pushButtonSupprimerUtilisateur_clicked()
{
  // TO DO
  MESSAGE m;
  m.type=1;
  strcpy(m.data1,getNom());
  m.requete=DELETE_USER;
  m.expediteur=getpid();
  fprintf(stderr,"(ADMIN %d) Attente d'une requete...\n",getpid());
  if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
    fprintf(stderr,"(ADMIN %d) erreur de msgsnd",getpid());
  }
  fprintf(stderr,"(ADMIN %d) envoi de la requete au serveur\n",getpid());
  if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
  {
    perror("(ADMIN) Erreur de msgrcv");
  }
  if(strcmp(m.data1,"OK")==0){
    dialogueMessage("retrait",m.texte);
  }
  else{
    dialogueErreur("retrait",m.texte);
  }


}

void WindowAdmin::on_pushButtonAjouterPublicite_clicked()
{
  // TO DO
  MESSAGE m;
  m.type=1;
  sprintf(m.data1, "%d", getNbSecondes());
  strcpy(m.texte,getTexte());
  //printf("%s,%s\n",m.data1,m.texte);
  m.requete=NEW_PUB;
  m.expediteur=getpid();
  fprintf(stderr,"(ADMIN %d) Attente d'une requete...\n",getpid());
  if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
    fprintf(stderr,"(ADMIN %d) erreur de msgsnd",getpid());
  }
}

void WindowAdmin::on_pushButtonQuitter_clicked()
{
  // TO DO
  MESSAGE m;
  //strcpy(m.data1,getNbSecondes());
  m.type=1;
  m.requete=LOGOUT_ADMIN;
  m.expediteur=getpid();
  if(msgsnd(idQ,&m,sizeof(MESSAGE)-sizeof(long),0)==-1){
    fprintf(stderr,"(CLIENT %d) erreur de msgsnd",getpid());
  }
  exit(0);
}
