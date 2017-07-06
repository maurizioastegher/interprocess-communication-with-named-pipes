#ifndef CRIPTADECRIPTA_H
#define CRIPTADECRIPTA_H

#include <stdio.h>

void creaFinalKey(char* secret_key, int secret_keySize, char *stringaCasuale, int  bytesCasualiLetti, char* risultato);
void cripta(char *risultato, int risultatoSize, char*testoLetto, int testoSize, char* testoCriptato);
void decripta(char *risultato, int risultatoSize, char*testoCriptato, int testoCriptatoSize, char* testoDecriptato);

#endif
