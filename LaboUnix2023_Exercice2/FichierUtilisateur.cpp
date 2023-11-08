#include "FichierUtilisateur.h"
#include <string.h>

int estPresent(const char* nom)
{
  int i=0;
  int fp;
  UTILISATEUR util;
  if((fp=open(FICHIER_UTILISATEURS,O_RDONLY))==-1){
    perror("Erreur de open()");
    return -1;
  }
  else{
    while(read(fp,&util,sizeof(util))>0){
      if(strcmp(nom,util.nom)==0){
        return i+1;
      }
      i++;
    }
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////////
int hash(const char* motDePasse)
{
  int i,s=0;
  for(i=1;i<strlen(motDePasse)+1;i++,motDePasse++){
    s=s+i*(int)*motDePasse;
  }
  return s%97;
}

////////////////////////////////////////////////////////////////////////////////////
void ajouteUtilisateur(const char* nom, const char* motDePasse)
{
  int fp;
  UTILISATEUR util;
  strcpy(util.nom,nom);
  util.hash=hash(motDePasse);
  if((fp=open(FICHIER_UTILISATEURS,O_WRONLY|O_CREAT|O_APPEND,0644))==-1){
    perror("1.Erreur de open()");


  }
  else{
    write(fp,&util,sizeof(util));
  }
}

////////////////////////////////////////////////////////////////////////////////////
int verifieMotDePasse(int pos, const char* motDePasse)
{
  UTILISATEUR util;
  int fp;
  if((fp=open(FICHIER_UTILISATEURS,O_RDONLY))==-1){
    perror("Erreur de open()");
    return -1;
  }
  else{
    lseek(fp,(pos-1)*sizeof(util),SEEK_SET);
    read(fp,&util,sizeof(util));
    printf("util.nom=%s   util.hash=%d   hash(motDePasse)=%d",util.nom,util.hash,hash(motDePasse));
    if(util.hash==hash(motDePasse)){
      return 1;
    }
  }
  return 0;
}



////////////////////////////////////////////////////////////////////////////////////
int listeUtilisateurs(UTILISATEUR *vecteur) // le vecteur doit etre suffisamment grand
{
  UTILISATEUR util;
  int fp,i=0;
  if((fp=open(FICHIER_UTILISATEURS,O_RDONLY))==-1){
    perror("Erreur de open()");
    return -1;
  }
  else{
    while(read(fp,&util,sizeof(util))>0)
    {
      strcpy(vecteur->nom,util.nom);
      vecteur->hash=util.hash;
      vecteur++;
      i++;
    }
  }
  return i;
}
