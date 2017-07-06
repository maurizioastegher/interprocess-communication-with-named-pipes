#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<fcntl.h>
#include<signal.h>
#include "../common/dati_ingresso.h"
#include "../common/criptaDecripta.h"
#include "../common/criptaDecripta.h"

#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) //lettura e scrittura per owner, lettura per gruppo e altri

void creaFinalKey(char* secret_key, int secret_keySize, char *stringaCasuale, int  bytesCasualiLetti, char* risultato){
  
  //faccio lo XOR bit a bit tra secret_key e la stringa da /dev/random
  //se ho prelevato meno bytes della lunghezza di s.key
  // --> ricomincio daccapo
  int i,j;
  for( i=0, j=0; i<secret_keySize-1; i++, j++){
    if(j==bytesCasualiLetti){
	printf("%i\n",i);
	j=0;
    }
    risultato[i] = secret_key[i]^stringaCasuale[j];

  }

  risultato[secret_keySize-1]='\0';
  return;
}

void cripta(char *risultato, int risultatoSize, char* testoLetto, int testoSize, char* testoCriptato){
 //faccio lo XOR bit a bit tra la key finale e il testo di input, metto testo risultante in 'testo criptato'
  //se ho key finale < testo di input
  // --> ricomincio dall'inizio
  int i,j;
  for( i=0, j=0; i<testoSize; i++, j++){
    if(j == risultatoSize-1)
      j=0;
    testoCriptato[i] = testoLetto[i]^risultato[j];
  }
  testoCriptato[testoSize]='\0';
  return;
}

void decripta(char *risultato, int risultatoSize, char*testoCriptato, int testoCriptatoSize, char* testoDecriptato){
 //faccio lo XOR bit a bit tra  key finale e il testo di input per la decifrazione
  //se ho key finale < testo criptato
  // --> ricomincio dall'inizio
	int i,j;
  for( i=0, j=0; i<testoCriptatoSize; i++, j++){
    if(j == risultatoSize-1)
      j=0;
    testoDecriptato[i] = testoCriptato[i]^risultato[j];
  }
	testoDecriptato[testoCriptatoSize]='\0';
	//printf("****testo decriptato: %s\n", testoDecriptato);
	return;
}
