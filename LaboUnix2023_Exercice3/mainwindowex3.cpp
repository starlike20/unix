#include "mainwindowex3.h"
#include "ui_mainwindowex3.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>

MainWindowEx3::MainWindowEx3(QWidget *parent):QMainWindow(parent),ui(new Ui::MainWindowEx3)
{
    ui->setupUi(this);
    fd = open("Trace.log", O_RDWR | O_CREAT, S_IRUSR | S_IWUSR);
    if (fd == -1) {
      perror("Erreur lors de l'ouverture du fichier");
    }
    else{
      dup2(fd, 2);
    }
}

MainWindowEx3::~MainWindowEx3()
{
    ::close(fd);
    ::close(2);
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void MainWindowEx3::setGroupe1(const char* Text)
{
  fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditGroupe1->clear();
    return;
  }
  ui->lineEditGroupe1->setText(Text);
}

void MainWindowEx3::setGroupe2(const char* Text)
{
  fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditGroupe2->clear();
    return;
  }
  ui->lineEditGroupe2->setText(Text);
}

void MainWindowEx3::setGroupe3(const char* Text)
{
  fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditGroupe3->clear();
    return;
  }
  ui->lineEditGroupe3->setText(Text);
}

void MainWindowEx3::setResultat1(int nb)
{
  char Text[20];
  sprintf(Text,"%d",nb);
  fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditResultat1->clear();
    return;
  }
  ui->lineEditResultat1->setText(Text);
}

void MainWindowEx3::setResultat2(int nb)
{
  char Text[20];
  sprintf(Text,"%d",nb);
  fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditResultat2->clear();
    return;
  }
  ui->lineEditResultat2->setText(Text);
}

void MainWindowEx3::setResultat3(int nb)
{
  char Text[20];
  sprintf(Text,"%d",nb);
  fprintf(stderr,"---%s---\n",Text);
  if (strlen(Text) == 0 )
  {
    ui->lineEditResultat3->clear();
    return;
  }
  ui->lineEditResultat3->setText(Text);
}

bool MainWindowEx3::recherche1Selectionnee()
{
  return ui->checkBoxRecherche1->isChecked();
}

bool MainWindowEx3::recherche2Selectionnee()
{
  return ui->checkBoxRecherche2->isChecked();
}

bool MainWindowEx3::recherche3Selectionnee()
{
  return ui->checkBoxRecherche3->isChecked();
}

const char* MainWindowEx3::getGroupe1()
{
  if (ui->lineEditGroupe1->text().size())
  { 
    strcpy(groupe1,ui->lineEditGroupe1->text().toStdString().c_str());
    return groupe1;
  }
  return NULL;
}

const char* MainWindowEx3::getGroupe2()
{
  if (ui->lineEditGroupe2->text().size())
  { 
    strcpy(groupe2,ui->lineEditGroupe2->text().toStdString().c_str());
    return groupe2;
  }
  return NULL;
}

const char* MainWindowEx3::getGroupe3()
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
void MainWindowEx3::on_pushButtonLancerRecherche_clicked()
{
  fprintf(stderr,"Clic sur le bouton Lancer Recherche\n");
  // TO DO
  int idFils1,idFils2,idFils3,status,id;
  if((recherche1Selectionnee()==1 && (idFils1=fork())!=-1)||(recherche1Selectionnee()==0 && recherche2Selectionnee()==0 &&recherche3Selectionnee()==0 && (idFils1=fork())!=-1)){
    if(idFils1==0){
      if(execl("./Lecture","lecture",getGroupe1(),NULL)==-1){
        fprintf(stderr,"Erreur de execl fils 1");
      }
    }
  }
  if(recherche2Selectionnee()==1 && (idFils2=fork())!=-1){
    if(idFils2==0){
      if(execl("./Lecture","lecture",getGroupe2(),NULL)==-1){
        fprintf(stderr,"Erreur de execl fils 2");
      }
    }
  }
  if(recherche3Selectionnee()==1 && (idFils3=fork())!=-1){
    if(idFils3==0){
      if(execl("./Lecture","lecture",getGroupe3(),NULL)==-1){
        fprintf(stderr,"Erreur de execl fils 1");
      }
    }
  }
  while((id=wait(&status))!=-1){
    if(WIFEXITED(status)){
      if(id==idFils1){
        setResultat1(WEXITSTATUS(status));
      }
      else{
        if(id==idFils2){
          setResultat2(WEXITSTATUS(status));
        }
        else{
          setResultat3(WEXITSTATUS(status));
        }
      }
    }
  } 
}

void MainWindowEx3::on_pushButtonVider_clicked()
{
  fprintf(stderr,"Clic sur le bouton Vider\n");
  // TO DO
  ui->lineEditGroupe1->clear();
  ui->lineEditGroupe2->clear();
  ui->lineEditGroupe3->clear();
  ui->lineEditResultat1->clear();
  ui->lineEditResultat2->clear();
  ui->lineEditResultat3->clear();
}

void MainWindowEx3::on_pushButtonQuitter_clicked()
{
  fprintf(stderr,"Clic sur le bouton Quitter\n");
  
  // TO DO
  exit(0);
} 
