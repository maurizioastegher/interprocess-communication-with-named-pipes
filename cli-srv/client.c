#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <fcntl.h>
#include "../common/dati_ingresso.h"
#include "../common/log.h"
#include "client.h"


int main(int argc, char** argv){
	
	if(argc != 5){
		char *exe = argv[0];
		printf("Usage: %s -e/-d \"secret_key\" testo_in chiaro.txt output.txt\n", exe);
		exit(0);
		}
	printf("\nCLIENT\n\n");
        creaLog(1);
	InputData* datiInvio = (InputData *)malloc(sizeof(InputData));//creo un'istanza della struct, che conterrà i dati da inviare al server
	discrimina_op(argc, argv, datiInvio);		//decide se criptare o decriptare
	strcpy(datiInvio->secret_key,argv[2]);  	//copia in struct s.key
	strcpy(datiInvio->testo,argv[3]);			//copio in struct il nome del file
	datiInvio->clientPid=getpid();
	
	if (datiInvio->op == 3)
		{
			char *stringaInput=(char*)malloc(DIM);
			printf("OBBLIGATORIA SCELTA DI ACQUISIZIONE INPUT DA FILE\ndigita nome file .txt: \n");
			scanf("%s",stringaInput);
			copia_file(stringaInput, argv[3]);
			int testoFD;
  			//APRO l'input da cui prendo il testo
  			if((testoFD = open(stringaInput, O_RDONLY))<0){ 
				logMessage("Immesso nome file non esistente, temina",1);
    				perror("ERRORE: impossibile aprire il file");
    				exit(0);
  			}
			logMessage("Apertura del file criptato avvenuta con successo",0);
  			bytesDaLeggere = (int)lseek(testoFD, 0L, SEEK_END); 
						//lseek ritorna il numero di bytes letti da testo da 0 alla fine(e sposta il puntatore alla fine)
			close(testoFD);
			logMessage("file con testo in chiaro creato con successo.",0);
			printf("Il documento %s e' stato copiato in %s.\n",stringaInput,argv[3]);}
	else 
		acquisisci_dati(datiInvio, argv[3]);		//chiedo allo user se vuole inserire dati da terminale o da file
								//da lì crea il file contenente il testo decriptato
	int fifoFD, clientPID, retfifoFD; 			//fifoFD è l'fd della FIFO pubblica dal client al server, retfifo della FIFO di ritorno
	char messaggioLettoFIFO[MAXDIM];
	if((fifoFD=open("FIFO",O_WRONLY))<0){ 		//Apre la FIFO pubblica in scrittura
		perror("ERRORE: impossibile aprire FIFO in scrittura");
		logMessage("impossibile aprire FIFO in scrittura",1);
		exit(0);
	}
	else logMessage("apertura FIFO pubblica in scrittura avvenuta con successo",0);
       
	clientPID = getpid();						//salvo il pid del client per poterlo inviare al server
	write(fifoFD, datiInvio, sizeof(InputData)); 	
												//Invia il messaggio al server contenente il suo pid	
	close(fifoFD); 								//Chiude la FIFO pubblica, comunicazione terminata

	char f[DIM]="";
	sprintf(f,"FIFO%d\0",clientPID); 			//Setta il nome della client-FIFO con il proprio pid
	//printf("nome fifo client: %s",f);	

	signal(SIGILL, handler);
	while((retfifoFD=open(f,O_RDONLY))<0);	//Attende finchè la client-FIFO non è creata e la apre
	
	if(read(retfifoFD, messaggioLettoFIFO, bytesDaLeggere)<0) 	//Legge dalla client-FIFO il risultato dell'operazione richiesta
		{perror("ERRORE: impossibile aprire RETFIFO in lettura");
		logMessage("impossibile aprire RETFIFO in lettura",0);
		}		
	else logMessage("apertura RETFIFO in lettura avvenuta con successo",0);		
	crea_file(messaggioLettoFIFO, argv[4]);
	printf("Il documento contenente il messaggio ricevuto %s è stato creato.\n",argv[4]);
	logMessage("documento con testo elaborato dal server creato con successo",0);
	close(retfifoFD); //Chiude la client-FIFO
}

void discrimina_op(int argc, char **argv, InputData* id)
{
	int scelta =0;
        while ((scelta = getopt (argc, argv, "e:d")) != -1){
	switch (scelta)
		{
			case 'e':
				printf("--> selezionata operazione di CRIPTAGGIO (-e)\n");
				logMessage("selezionata operazione di CRIPTAGGIO",0);
				id->op = 2;
				break;
			case 'd':
				printf("--> selezionata operazione di DECRIPTAGGIO (-d)\n");
				logMessage("selezionata operazione di DECRIPTAGGIO",0);
				id->op = 3;
				break;
			default:
				printf("ERRORE: Opzione non conosciuta\n");
				logMessage("selezionata operazione errata, termina",1);
				exit(0);
		}}
     	return;
}

void handler(int sig){
	printf("Il server NON può soddisfare la richiesta, non ha sufficienti informazioni!\n");
	logMessage("non è possibile procedere con la decript, il server non dispone della chiave, termina",01);
	exit(0);
}

void acquisisci_dati(InputData* id, char* nome)
{
	printf("In che modo desideri acquisire i dati? \n(1)\tda terminale\n(2)\tda file\n");		
	int scelta =0;
	//char *stringaInput=(char*)malloc(100000) ;	
	char stringaInput [DIM];
	scanf("%d",&scelta);
	
	switch (scelta){	
		case 1:{
			printf("DA TERMINALE\ndigita il tuo testo terminando con invio: \n");
			//fflush(stdin); 
			scanf("\n%100[^\n]s",stringaInput);			//!!!!!!! 100 dovrebbe essere DIM! [ma non lo prende]
			//printf("striga letta: %s", stringaInput); 
			//scanf("%s",stringaInput);		
			//gets( stringaInput );			
			bytesDaLeggere = sizeof(stringaInput);
			crea_file(stringaInput,nome);
			printf("doc %s creato.\n",nome);
			logMessage("acquisizione dati di input da terminale.",0);
			}break;
		case 2:
			printf("DA FILE\ndigita nome file .txt: \n");
			scanf("%s",stringaInput);
			copia_file(stringaInput,nome);

			int testoFD;
  			
  			if((testoFD = open(stringaInput, O_RDONLY))==-1){	//apro l'input da cui prendo il testo 
    				perror("ERRORE: impossibile aprire il file");
				logMessage("impossibile acquisizione input da file, termina.",1);
    				exit(0);
  			}
  			bytesDaLeggere = (int)lseek(testoFD, 0L, SEEK_END); 
						//lseek ritorna il numero di bytes letti da testo da 0 alla fine(e sposta il puntatore alla fine)
			close(testoFD);
			printf("Il documento %s e' stato copiato in %s.\n",stringaInput,nome);
			logMessage("acquisizione dati di input da file avvenuta con successo",0);
			break;
		default:
			printf("ERRORE: Opzione non conosciuta\n");
			logMessage("scelta acquisizione dati di input non valida, termina",1);
			exit(0);
		}
}

void crea_file(char *stringa, char *nome)
{	
  	int fileInputFD, fileScrittiTxt;
    fileInputFD = open(nome, O_WRONLY| O_CREAT| O_TRUNC, FILE_MODE);
    if(fileInputFD < 0){
        perror("ERRORE: impossibile aprire il file");
	logMessage("Impossibile aprire il file, termina",1);
		exit(0);
	}
    else logMessage("file aperto con successo",0);
    fileScrittiTxt = write(fileInputFD, stringa, bytesDaLeggere);
	if (fileScrittiTxt<0){
		perror("ERRORE: impossibile scrivere sul file");
		logMessage("Impossibile scrivere su file.",1);
		exit(0);
	}
	else logMessage("scrittura su file avvenuta con successo",10);
     return;
}

void copia_file(char *nome, char *copia)
{
	int infile, outfile;
	int nr;
	char buff [BUFSIZ];
	infile = open(nome, O_RDONLY | O_CREAT, FILE_MODE);
	if (infile<0){
		perror("ERRORE: impossibile aprire il file");
		logMessage("Impossibile aprire il file, termina",1);
		exit(0);
	}
	else logMessage("file aperto con successo",0);
	outfile = creat(copia,FILE_MODE);
	while((nr = read(infile, buff, BUFSIZ)) > 0)
		write (outfile, buff, nr);
	close (infile);
	close (outfile);
}
