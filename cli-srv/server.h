
#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) //lettura e scrittura per owner, lettura per gruppo e altri

void server(InputData* datiRicevuti, int fifoFD, int* final_keyCreata, char *risultato);
int leggiInputDaFile(char* fileName, char* testoLetto);
int devrandom(char *stringaCasuale, int secret_keySize);

