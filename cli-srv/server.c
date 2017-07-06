#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>

#include "../common/dati_ingresso.h"
#include "../common/criptaDecripta.h"
#include "../common/log.h"
#include "server.h"


int main(){
	creaLog(0);
	int fifoFD;
	int pidc;
	char risposta='n';
	int final_keyCreata = 0;
	char risultato[DIM];
	InputData* datiRicevuti = (InputData *)malloc(sizeof(InputData)); 
	if(mkfifo("FIFO",S_IRWXU|S_IRGRP|S_IROTH)<0){ //crea FIFO
		perror("Impossibile creare la FIFO");
		logMessage("Impossibile creare la FIFO",1);
		exit(0);
	}
	printf("\nSERVER\n\n");
	while(risposta != 'y'){
		logMessage("server operativo",0);
		if((fifoFD=open("FIFO",O_RDONLY))<0){ //Apre la well-known FIFO
			perror("Error: impossibile aprire la FIFO in sola lettura");
			logMessage("Impossibile aprire la FIFO in sola lettura",1);
			exit(0);
		}
		server(datiRicevuti, fifoFD, &final_keyCreata, risultato); //Se non incorre in errori segue le operazioni richieste
		close(fifoFD); //Chiude la FIFO 'pubblica'
		printf("Vuoi chiudere il server perdendo la tua secret key? (y / n) \n");
		fflush(stdin); 
    	        scanf(" %c",&risposta);
	}
	logMessage("Esecuzione server terminata",0);
	unlink("FIFO"); //Rimuove la well-known FIFO
	//unlink("RETFIFO");
	exit(0);
}

void server(InputData* datiRicevuti, int fifoFD, int *final_keyCreata, char* risultato){
	int pidClient, retFifoFD;
	logMessage("inizio elaborazione server",0);
	if(read(fifoFD, datiRicevuti, sizeof(InputData))<0){ //Legge dalla well-known FIFO
		perror("Error: il messaggio non è valido");
		logMessage("Messaggio ricevuto dalla fifo non valido",1);
		return;
	}
	logMessage("messaggio ricevuto dal client con successo",0);
	printf("\nIl server ha letto dalla FIFO: \noperazione richiesta--> %d\nsecret_keyk--> %s\ninput--> %s\npid client--> %d\n", datiRicevuti->op, datiRicevuti->secret_key, datiRicevuti->testo, datiRicevuti->clientPid);
	if((datiRicevuti->op==3)&&(*final_keyCreata==0)){
		kill(datiRicevuti->clientPid, 4);
		logMessage("Non ho la chiave necessaria!",1);
		printf("NON posso soddisfare la richiesta! Non ho la chiave necessaria!\n");
		return;			
	}
	logMessage("richiesta di elaborazione del testo valida",0);
	int secret_keySize = strlen(datiRicevuti->secret_key);
	char testoLetto[MAXDIM];
	int testoSize;
	testoSize = leggiInputDaFile(datiRicevuti->testo, testoLetto);	
	//printf("testoSize:%i\n",testoSize);
	char stringaCasuale[secret_keySize+1]; //HA /0
	int bytesCasualiLetti = devrandom(stringaCasuale, secret_keySize);
	stringaCasuale[bytesCasualiLetti]= '\0';

	//********************CRIPTO O DECRIPTO*****************************************
	
	char risultatoInterno[secret_keySize];
	char soluzione[testoSize+1];

	if(*final_keyCreata==0){
		creaFinalKey(datiRicevuti->secret_key, strlen(datiRicevuti->secret_key), stringaCasuale, bytesCasualiLetti, risultatoInterno);	
		strcpy(risultato, risultatoInterno);	
		*final_keyCreata = 1;	
		logMessage("Final key creata",0);
	}
	
	switch (datiRicevuti->op) //piloto l'esecuzione del programma relativamente al valore di op
	  {
	  case 2:
		printf("... criptaggio ...\n");
		logMessage("Inizio criptaggio",0);
		cripta(risultato, secret_keySize, testoLetto, testoSize, soluzione);
		logMessage("Il server ha ultimato il criptaggio",0);
		printf("-->criptaggio completato!\n");
	    	break;
	 case 3:
	 	printf("... decriptaggio ...\n");
		logMessage("Inizio decriptaggio",0);
	 	decripta(risultato, secret_keySize, testoLetto, testoSize, soluzione);
		logMessage("Il server ha ultimato il decriptaggio",0);
		printf("-->decriptaggio completato!\n");
		break;
	default:
		printf("Unknown option character\n");
		logMessage("Unknown option character",1);
		exit(0);
	  }
	
	//**********************************************************************
	//sprintf(f,"FIFO%d\0",pid); //Compone il nome della client-FIFO, FIFO+pid

	char f[DIM]="";
	sprintf(f,"FIFO%d\0",datiRicevuti->clientPid); 			//Setta il nome della client-FIFO con il proprio pid
	printf("--> nome RETFIFO : %s\n",f);   

	if(mkfifo(f,S_IRWXU|S_IRGRP|S_IROTH)<0){ 			//Crea la client-FIFO
		perror("impossibile creare la RETFIFO");
		logMessage("impossibile creare la RETFIFO",1);
		return;
		}
	else logMessage("client-fifo creata con successo",0);
	if((retFifoFD = open(f,O_WRONLY))<0){ 				//Apre la client-FIFO
		perror("Impossibile aprire la RETFIFO");
		logMessage("Impossibile aprire la RETFIFO",1);
		return;
		}
	logMessage("client-fifo aperta con successo",0);

	if(write(retFifoFD, soluzione, testoSize+1)<0) 	{			//Scrive nella client-FIFO
		perror("ERRORE: Impossibile scrivere");
		logMessage("Impossibile aprire la RETFIFO in scrittura",1);}
	else logMessage("RETFIFO aperta in scrittura con successo",0);
	close(retFifoFD); 											//Chiude la client-FIFO
	unlink(f); 											//Rimuove la client-FIFO
	
	return;
}

int devrandom(char *stringaCasuale, int secret_keySize){
	int randomFD;
	if((randomFD = open("/dev/random", O_RDONLY))==-1){			//apro /dev/random
		perror("errore di apertura di /dev/random");
		logMessage("errore di apertura di /dev/random",1);
		exit(0);
	}
	else logMessage("accesso alla dev/random avvenuta con successo",0);
	
	int bytesCasualiLetti=read(randomFD, &stringaCasuale, secret_keySize);		
	if(bytesCasualiLetti == -1){					//leggo da /dev/random e salvo in stringa; se dà errore esco
	  perror("errore di lettura da file");	
		logMessage("errore lettura da file",1);							
	  exit(0);											
	}
	else logMessage("lettura dei byte casuali avvenuta con successo",0);
		
	if(close(randomFD)==-1){									//chiudo /dev/random
	  perror("errore di chiusura");
	logMessage("errore di chiusura fd",1);
	  exit(0);
	}

	return bytesCasualiLetti;									//ritorno il numero di bytes effettivamente letti
}

 //***************INPUT FILE**********************************

int leggiInputDaFile(char* fileName, char* testoLetto){
 
	int testoFD;	  //APRO l'input da cui prendo il testo
  	if((testoFD = open(fileName, O_RDONLY))==-1){ 
  		perror("errore di apertura del file");
		logMessage("errore di apertura del file",1);
    		exit(0);
  		}
	logMessage("lettura dell'input avvenuta con successo",0);
  	int testoSize = (int)lseek(testoFD, 0L, SEEK_END); //lseek ritorna il numero di bytes letti da testo da 0 alla fine(e sposta il puntatore alla fine)
								
  char testo[testoSize]; 

  	if(testoSize == -1){
   		perror(fileName);
    	exit(0);
  	}

  	printf("testosize: %i\n", testoSize);
  
  	//posiziono offset a 0 e leggo da inputFile (sposta il puntatore del file all'inizio)
  	lseek(testoFD, 0L, SEEK_SET);
  	int bytesInputLetti = read(testoFD, &testo, testoSize);
  
  	//printf("ho letto da testo %i bytes\n", bytesInputLetti);
  	if(bytesInputLetti==-1){
    	perror("lettura testo");
		logMessage("errore di lettura del testo",1);
    	exit(0);
  	}
  
 	strcpy(testoLetto, testo);

	close(testoFD);
  
	return bytesInputLetti;
}
