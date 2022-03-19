#ifndef __MYCODE_H__
#define __MYCODE_H__

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <errno.h>
#include <dirent.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <math.h>

#include "fila.h"

#define LIST_COMMAND 0
#define STATS_COMMAND 1
#define SEND_COMMAND 2
#define ERROR_COMMAND 100
#define MAX_BUFFERSIZE 500


typedef struct arg_send_ {
    char *filename;
    char *filesdirectory;
    int nfiles;
    int findex;
    int socket;
} arg_send;

typedef struct arg_read_ {
    int nfiles;
    char *dirdestiny;
    int socket;
} arg_read;

typedef struct arg_s {
    int communicationsocket;
    int portnum;
    int nfiles;
    char *filesdirectory;
    int partssockets[100];
} argumento_s;

typedef struct args_c {
    int communicationsocket;
    int portnum;
    int nfiles;
    char *clientrepository;
    char *filename;
    int partssockets[100];
} argumentos_c;



void sendInt( int n, int socket );
int readInt( int socket );

void sendDouble( double n, int socket );
double readDouble( int socket );

void sendStr( char *str, int socket );
char *readStr( int socket );

void sendLong( long n, int socket );
long readLong( int socket );

void sendVoid( void *v, int numofbytes, int socket );
void *readVoid( int socket );

void listFiles(const char* dirname, QUEUE *fila);

void sendFileNames( QUEUE *fila, int socket );

void sendstats( time_t init, int filesent, long bytesent, char *filesdirectory, int socket );

void *sendfilepart( void *ptr );

void *readfilepart( void *ptr );

long returnFileSize_s( char *filename );

long returnFileSize_c( char *filename );

void *multithread_s(void *ptr);

void *multithread_c(void *ptr);

#endif
