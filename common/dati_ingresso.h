#define DIM 100
#define MAXDIM 1024
#define DEF_NUM 900

typedef struct dati_ingresso{
  	int op;    //tipo di operazione
	int clientPid;
  	char secret_key[DIM];
  	char testo[MAXDIM];
} 	InputData;
