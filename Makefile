all: client server clean 


client: fila.o biblioteca.o client.o
	clang -pthread -o client fila.o biblioteca.o client.o

server: fila.o biblioteca.o server.o
	clang -pthread -o server fila.o biblioteca.o server.o

client.o: client.c
	clang -c -g client.c

server.o: server.c
	clang -c -g server.c

fila.o: fila.c 
	clang -c -g fila.c

biblioteca.o: biblioteca.c
	clang -c -g biblioteca.c

clean: 
	rm *.o