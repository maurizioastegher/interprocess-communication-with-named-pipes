#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#define LOG_PATH "/var/log/cli_srv/"
#define SRV_LOG "server.log"
#define CLI_LOG "client.log"
#define L_SRV 0
#define L_CLI 1
#define LOG_SIZE 130

void logMessage(char msg[], int tipo);
void creaLog(int tipo);
FILE *logfile;
int kind = -1;

void creaLog(int tipo){

    char str[LOG_SIZE];
    strcpy(str, LOG_PATH);    
    if(tipo == 0){          //init SERVER log file		
      if(mkdir("/var/log/cli_srv/", S_IRWXU | S_IRWXG | S_IRWXO ) != 0){  //creazione della cartella se non esistente
	perror("creazione cartella -->");
        if(chmod("/var/log/cli_srv/", S_IRWXU | S_IRWXG | S_IRWXO) != 0){
	  perror("chmod -->");
          exit(0);
        }
      }
      strcat(str, SRV_LOG);
      logfile = fopen(str, "a+");
	logMessage("log file creato", 0);
      if(logfile < 0){
        printf("Cannot OPEN server-log file -> EXIT PROGRAM\n");
        exit(0);
      }
      fprintf(logfile,"********************************SERVER LOG FILE********************************\n\n\n");
       
    }

    if(tipo == 1){		//init CLIENT log file	
      strcat(str, CLI_LOG);
      logfile = fopen(str, "a+");
	logMessage("log file creato", 0);
      if(logfile < 0){
	perror("open client.log ");
        exit(0);
      }
      fprintf(logfile,"********************************CLIENT LOG FILE********************************\n\n\n"); 
    
    }
  }
  


void logMessage(char msg[LOG_SIZE], int tipo){
	
	char *timestamp = (char *)malloc(sizeof(char) * 16);
	time_t ltime = time(NULL);
	struct tm *tm;
	tm=localtime(&ltime);
	if (tipo== 1){
		fprintf(logfile,"%02d-%02d-%04d %02d:%02d:%02d -> ERRORE: %s\n",tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900,tm->tm_hour, tm->tm_min, tm->tm_sec ,msg);}
	if (tipo== 0){
		fprintf(logfile,"%02d-%02d-%04d %02d:%02d:%02d -> INFO: %s\n",tm->tm_mday, tm->tm_mon+1, tm->tm_year+1900,tm->tm_hour, tm->tm_min, tm->tm_sec ,msg);}
}



