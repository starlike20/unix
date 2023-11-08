#include "mainwindowex4.h"
#include "ui_mainwindowex4.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>

extern MainWindowEx4 *w;

int idFils1, idFils2, idFils3;
void HandlerSIGCHLD(int);

struct sigaction A;

// Armement de SIGCHLD

// TO DO : HandlerSIGCHLD

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
MainWindowEx4::MainWindowEx4(QWidget *parent):QMainWindow(parent),ui(new Ui::MainWindowEx4)
{
  ui->setupUi(this);
  ui->pushButtonAnnulerTous->setVisible(false);

  // armement de SIGCHLD
  A.sa_handler = &HandlerSIGCHLD;
  sigemptyset(&A.sa_mask);
  A.sa_flags = 0;
  if (sigaction(SIGCHLD,&A,NULL) == -1)
  {
    perror("Erreur de sigaction");
    exit(1);
  }
  // TO DO
  fd = open("Trace.log", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    perror("Erreur lors de l'ouverture du fichier");
  }
  else{
    dup2(fd, 2);
  }
}

MainWindowEx4::~MainWindowEx4()
{
  ::close(fd);
  ::close(2);
  delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowEx4::setGroupe1(const char* Text)
{
  //fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditGroupe1->clear();
    return;
  }
  ui->lineEditGroupe1->setText(Text);
}

void MainWindowEx4::setGroupe2(const char* Text)
{
  //fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditGroupe2->clear();
    return;
  }
  ui->lineEditGroupe2->setText(Text);
}

void MainWindowEx4::setGroupe3(const char* Text)
{
  //fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditGroupe3->clear();
    return;
  }
  ui->lineEditGroupe3->setText(Text);
}

void MainWindowEx4::setResultat1(int nb)
{
  char Text[20];
  sprintf(Text,"%d",nb);
  //fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditResultat1->clear();
    return;
  }
  ui->lineEditResultat1->setText(Text);
}

void MainWindowEx4::setResultat2(int nb)
{
  char Text[20];
  sprintf(Text,"%d",nb);
  //fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditResultat2->clear();
    return;
  }
  ui->lineEditResultat2->setText(Text);
}

void MainWindowEx4::setResultat3(int nb)
{
  char Text[20];
  sprintf(Text,"%d",nb);
  //fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditResultat3->clear();
    return;
  }
  ui->lineEditResultat3->setText(Text);
}

bool MainWindowEx4::traitement1Selectionne()
{
  return ui->checkBoxTraitement1->isChecked();
}

bool MainWindowEx4::traitement2Selectionne()
{
  return ui->checkBoxTraitement2->isChecked();
}

bool MainWindowEx4::traitement3Selectionne()
{
  return ui->checkBoxTraitement3->isChecked();
}

const char* MainWindowEx4::getGroupe1()
{
  if (ui->lineEditGroupe1->text().size())
  { 
    strcpy(groupe1,ui->lineEditGroupe1->text().toStdString().c_str());
    return groupe1;
  }
  return NULL;
}

const char* MainWindowEx4::getGroupe2()
{
  if (ui->lineEditGroupe2->text().size())
  { 
    strcpy(groupe2,ui->lineEditGroupe2->text().toStdString().c_str());
    return groupe2;
  }
  return NULL;
}

const char* MainWindowEx4::getGroupe3()
{
  if (ui->lineEditGroupe3->text().size())
  { 
    strcpy(groupe3,ui->lineEditGroupe3->text().toStdString().c_str());
    return groupe3;
  }
  return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowEx4::on_pushButtonDemarrerTraitements_clicked()
{
  fprintf(stderr,"Clic sur le bouton Demarrer Traitements\n");
  // TO DO

  if((traitement1Selectionne()==1 && (idFils1=fork())!=-1)||(traitement1Selectionne()==0 && traitement2Selectionne()==0 &&traitement3Selectionne()==0 && (idFils1=fork())!=-1)){
    if(idFils1==0){
      if(execl("./Traitement","Traitement",getGroupe1(),"200",NULL)==-1){
        fprintf(stderr,"Erreur de execl fils 1");
      }
    }
  }
  if(traitement2Selectionne()==1 && (idFils2=fork())!=-1){
    if(idFils2==0){
      if(execl("./Traitement","Traitement",getGroupe2(),"450",NULL)==-1){
        fprintf(stderr,"Erreur de execl fils 2");
      }
    }
  }
  if(traitement3Selectionne()==1 && (idFils3=fork())!=-1){
    if(idFils3==0){
      if(execl("./Traitement","Traitement",getGroupe3(),"700",NULL)==-1){
        fprintf(stderr,"Erreur de execl fils 1");
      }
    }
  }
}

void MainWindowEx4::on_pushButtonVider_clicked()
{
  fprintf(stderr,"Clic sur le bouton Vider\n");
  // TO DO
  setGroupe1(" ");
  setGroupe2(" ");
  setGroupe3(" ");
  setResultat1(0);
  setResultat2(0);
  setResultat3(0);
}

void MainWindowEx4::on_pushButtonQuitter_clicked()
{
  fprintf(stderr,"Clic sur le bouton Quitter\n");
  // TO DO
  exit(0);
}

void MainWindowEx4::on_pushButtonAnnuler1_clicked()
{
  fprintf(stderr,"Clic sur le bouton Annuler1\n");
  // TO DO
  kill(idFils1,SIGUSR1);
}

void MainWindowEx4::on_pushButtonAnnuler2_clicked()
{
  fprintf(stderr,"Clic sur le bouton Annuler2\n");
  // TO DO
  kill(idFils2,SIGUSR1);
}

void MainWindowEx4::on_pushButtonAnnuler3_clicked()
{
  fprintf(stderr,"Clic sur le bouton Annuler3\n");
  // TO DO
  kill(idFils3,SIGUSR1);
}

void MainWindowEx4::on_pushButtonAnnulerTous_clicked()
{
  // fprintf(stderr,"Clic sur le bouton Annuler tout\n");
  // NOTHING TO DO --> bouton supprimé
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// Handlers de signaux //////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////

// TO DO : HandlerSIGCHLD
void HandlerSIGCHLD(int Sig)
{
  int id, status;
  if((id=wait(&status))!=-1){
    if(WIFEXITED(status)){
      if(id==idFils1){
        w->setResultat1(WEXITSTATUS(status));
      }
      else{
        if(id==idFils2){
          w->setResultat2(WEXITSTATUS(status));
        }
        else{
          w->setResultat3(WEXITSTATUS(status));
        }
      }
    }
  } 
}