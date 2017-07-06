all: server client

client: client.o log.o
	gcc client.o log.o -o client

server: server.o log.o criptaDecripta.o
	gcc server.o log.o criptaDecripta.o -o server

criptaDecripta.o : criptaDecripta.c ../common/criptaDecripta.h
	gcc -c criptaDecripta.c	

client.o : client.c ../common/dati_ingresso.h ../common/log.h
	gcc -c client.c

server.o : server.c  ../common/log.h server.h ../common/criptaDecripta.h ../common/dati_ingresso.h
	gcc -c server.c

log.o : log.c ../common/log.h 
	gcc -c log.c 

clean: 
	rm *.o server client	

.PHONY: 
	clean 
