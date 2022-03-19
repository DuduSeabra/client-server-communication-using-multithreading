#include "biblioteca.h"
#include "fila.h"


#define MAXRCVLEN 500

typedef struct arg_c {
    char *type;
    char *clientdirectory;
    int socket;
} argumento_c;


int portnumaux[100];


void makeclientrequest(char* command, char* clientrepository, int fileqtd, int nfiles, int filessocket[], int *partssocket[],int comsocket)
{
    char **fileslist = (char **) calloc(10, sizeof(char *));
    for(int i = 0; i < 10; i++) {
        fileslist[i] = (char *) calloc(100, sizeof(char));
    }





    if(strcmp(command, "list") == 0) {

        sendInt(LIST_COMMAND, comsocket);
        int fileqtd = readInt(comsocket);

        char *c;

        printf("Lista de arquivos:\n%d\n", fileqtd);


        for(int i = 0; i < fileqtd; i++) {
            c = readStr(comsocket);
            printf("%s\n", c);
        }
   }

    else if(strcmp(command, "stats") == 0) {

        sendInt(STATS_COMMAND, comsocket);
        int hour = readInt(comsocket);
        int min = readInt(comsocket);
        int sec = readInt(comsocket);

        printf("Uptime: %d hour %d min %d sec\n", hour, min, sec);
        printf("-------------------------\n");

        int fileqtd = readInt(comsocket);
        int filesent = readInt(comsocket);
        long bytesent = readLong(comsocket);

        printf("Numero de arquivos no servidor: %d\n", fileqtd);
        printf("Numero de arquivos enviados: %d\n", filesent);
        printf("Numero de bytes enviados: %ld\n", bytesent);
        
    }

    else if(strcmp(command, "download") == 0) {

        sendInt(SEND_COMMAND, comsocket);
        printf("Files names: ");
        for(int i = 0; i < fileqtd; i++) {
            scanf("%s", fileslist[i]);
        }
        sendInt(nfiles, comsocket);
        sendInt(fileqtd, comsocket);
        int contador = readInt(comsocket);

        for(int i = 0; i < fileqtd; i++) {
            sendStr(fileslist[i], comsocket);
        }

        argumentos_c argumentos[fileqtd];
        pthread_t thread_c[fileqtd];

        int aux = 0;

        if(clientrepository[8] == '1') {
            aux = 100;
        }
        else if(clientrepository[8] == '2') {
            aux = 200;
        }
        else if(clientrepository[8] == '3') {
            aux = 300;
        }

        sendInt(aux, comsocket);

        for(int i = 0; i < fileqtd; i++) {
            int partaux[100];
            for(int j = 0; j < nfiles; j++) {
                argumentos[i].partssockets[j] = partssocket[i][j];
            }
            
            argumentos[i].communicationsocket = filessocket[i];
            argumentos[i].nfiles = nfiles;
            argumentos[i].clientrepository = (char*)calloc(strlen(clientrepository)+1, sizeof(char));
            strcpy(argumentos[i].clientrepository, clientrepository);
            argumentos[i].filename = (char*)calloc(strlen(fileslist[i])+1, sizeof(char));
            strcpy(argumentos[i].filename, fileslist[i]);
        }

        

        for(int i = 0; i < fileqtd; i++) {
            pthread_create(&thread_c[i], NULL, multithread_c, &argumentos[i]);
        }

        for(int i = 0; i < fileqtd; i++) {
            pthread_join(thread_c[i], NULL);
        }

    }

   else {
       sendInt(ERROR_COMMAND, comsocket);
   }
}


int main(int argc, char *argv[])
{

	if( argc != 4 ){

        printf("USAGE: server port_number\n");

        return EXIT_FAILURE;

    }
   int communicationsocket = 0;
   struct sockaddr_in dest;

   communicationsocket = socket(AF_INET, SOCK_STREAM, 0);

   memset(&dest, 0, sizeof(dest));                /* zero the struct */
   dest.sin_family = AF_INET;
   dest.sin_addr.s_addr = htonl(INADDR_LOOPBACK); /* set destination IP number - localhost, 127.0.0.1*/
   dest.sin_port = htons(  atoi( argv[ 3 ]) );                /* set destination port number */

    int fileqtd = 0;
    int nfiles = 0;

    
    int connectResult = connect(communicationsocket, (struct sockaddr *)&dest, sizeof(struct sockaddr_in));


   if( connectResult == - 1 ){

   	    printf("CLIENT ERROR: %s\n", strerror(errno));

   		return EXIT_FAILURE;
   }

   int filessockets[100];
   int *partssockets[100];

   for(int i = 0; i < 100; i++) {
       partssockets[i] = calloc(100, sizeof(int));
   }

    if(strcmp(argv[1], "download") == 0) {
        printf("Number of files: ");
        scanf("%d", &fileqtd);
        //printf("\n");
        printf("Number of parts: ");
        scanf("%d", &nfiles);
        sendInt(1, communicationsocket);
        sendInt(fileqtd, communicationsocket);
        sendInt(nfiles, communicationsocket);
        int contador = readInt(communicationsocket);

        for(int i = 0; i < fileqtd; i++) {
            int communicationsocket_c = socket(AF_INET, SOCK_STREAM, 0);
            struct sockaddr_in dest_c;
            memset(&dest_c, 0, sizeof(dest_c));
            dest_c.sin_family = AF_INET;
            dest_c.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
            dest_c.sin_port = htons(  ((i+7+contador)*1000) );

            //printf("client.c: %d\n", ((i+7+contador)*1000));

            int connectResult_c = -1;
            while(connectResult_c == -1) {
                connectResult_c = connect(communicationsocket_c, (struct sockaddr *)&dest_c, sizeof(struct sockaddr_in));
            }
            filessockets[i] = communicationsocket_c;


        }
        for(int i = 0; i < fileqtd; i++) {
            for(int j = 0; j < nfiles; j++) {
                int communicationsocket_c2 = socket(AF_INET, SOCK_STREAM, 0);
                struct sockaddr_in dest_c2;
                memset(&dest_c2, 0, sizeof(dest_c2));
                dest_c2.sin_family = AF_INET;
                dest_c2.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
                dest_c2.sin_port = htons(((i+7+contador)*1000)+1+j);

                int connectResult_c2 = -1;
                while(connectResult_c2 == -1) {
                    connectResult_c2 = connect(communicationsocket_c2, (struct sockaddr *)&dest_c2, sizeof(struct sockaddr_in));
                }
                partssockets[i][j] = communicationsocket_c2;
            }
        }
        printf("Successful connections\n");
    }
    else {
        sendInt(0, communicationsocket);
    }
    

   int aux = 5002;

    for(int i = 0; i < 100; i++) {
        portnumaux[i] = aux;
        aux++;
    }
   

   makeclientrequest(argv[1], argv[2], fileqtd, nfiles, filessockets, partssockets, communicationsocket); ///// mudar para pthread_create
   

   close(communicationsocket);
   return EXIT_SUCCESS;
}
