#define FILE_MODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

void discrimina_op(int argc, char** argv, InputData* id);
void acquisisci_dati(InputData* id, char *nome);
void crea_file(char *stringa, char *nome);
void copia_file(char *nome, char *copia);
void handler(int);

int bytesDaLeggere = 0;	
