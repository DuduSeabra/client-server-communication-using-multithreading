#include "biblioteca.h"
#include "fila.h"


typedef struct arg_t {
    time_t init;
    char *filesdirectory;
    int communicationsocket;
    int fileqtd;
    int nfiles;
    int filessockets[100];
    int partssockets[100][100];
} argumento;


int portnumaux_partes[100];
int portnumaux_principal[100];
int contador_principal;
int contador_partes;

int contador;
long bytesent;


void* processclientrequest(void* ptr)
{
    argumento argumentos = *((argumento*)ptr);
    int comsocket = argumentos.communicationsocket;
    int fileqtd = argumentos.fileqtd;
    int nfiles = argumentos.nfiles;
    int filessockets[100];
    int partsockets[100][100];
    for(int i = 0; i < fileqtd; i++) {
        filessockets[i] = argumentos.filessockets[i];
        for(int j = 0; j < nfiles; j++) {
            partsockets[i][j] = argumentos.partssockets[i][j];
        }
    }


    char *filesdirectory = argumentos.filesdirectory;
    time_t init = argumentos.init;
    int pedido = readInt(comsocket);
    QUEUE* fila = queue_create();

    switch(pedido) {
        case 0:
            listFiles(filesdirectory, fila);
            sendFileNames(fila, comsocket);
            break;
        case 1:
            sendstats(init, contador, bytesent, filesdirectory, comsocket);
            break;
        case 2:
        {
            int nfiles = readInt(comsocket);
            int fileqtd = readInt(comsocket);
            argumento_s argumentos_s[fileqtd];
            pthread_t thread_s[fileqtd];
            sendInt(contador, comsocket);

            for(int i = 0; i < fileqtd; i++) {
                char *fname = readStr(comsocket);
                char *auxname = calloc(80, sizeof(char));
                snprintf(auxname, 80, "%s/%s", filesdirectory, fname);
                long filesize = returnFileSize_s(auxname);
                bytesent = bytesent + filesize-1;
            }

            int aux1 = readInt(comsocket);

            for(int i = 0; i < fileqtd; i++) {
                
                for(int j = 0; j < nfiles; j++) {
                    argumentos_s[i].partssockets[j] = partsockets[i][j];
                }

                argumentos_s[i].communicationsocket = filessockets[i];

                argumentos_s[i].nfiles = nfiles;
                argumentos_s[i].filesdirectory = (char*)calloc(strlen(filesdirectory)+1, sizeof(char));
                strcpy(argumentos_s[i].filesdirectory, filesdirectory);
            }
            pthread_mutex_lock(&mutex_size3);
            //contador = contador + fileqtd;
            pthread_mutex_unlock(&mutex_size3);

            for(int i = 0; i < fileqtd; i++) {
                pthread_create(&thread_s[i], NULL, multithread_s, &argumentos_s[i]);
            }

            for(int i = 0; i < fileqtd; i++) {
                pthread_join(thread_s[i], NULL);
            }
            
            break;
        }

        default:
            printf("Opcao invalida\n");
    }

    return (void*) 0;
}



int main(int argc, char *argv[])
{

    char *filesdirectory;
    time_t init;

    if( argc != 3 ){ 

        printf("USAGE: server dir port_number\n");

        return EXIT_FAILURE;

    }
    filesdirectory = calloc(strlen(argv[1]) + 1, sizeof(char));
    filesdirectory = argv[1];
    int port_number = atoi( argv[ 2 ] );

    

    srand( time( NULL ));



    
    pthread_t thread[50];
    int count_thread = 0;


    struct sockaddr_in dest; /* socket info about the machine connecting to us */
    struct sockaddr_in serv; /* socket info about our server */
    int mysocket;            /* socket used to listen for incoming connections */
    socklen_t socksize = sizeof(struct sockaddr_in);

    memset(&serv, 0, sizeof(serv));           /* zero the struct before filling the fields */
    serv.sin_family = AF_INET;                /* set the type of connection to TCP/IP */
    serv.sin_addr.s_addr = htonl(INADDR_ANY); /* set our address to any interface */
    serv.sin_port = htons( port_number );           /* set the server port number */

    mysocket = socket(AF_INET, SOCK_STREAM, 0);

    /* bind serv information to mysocket */
    bind(mysocket, (struct sockaddr *)&serv, sizeof(struct sockaddr));

    /* start listening, allowing a queue of up to 1 pending connection */
    listen(mysocket, 10);

    init = time(NULL);


    printf("Server is waiting for connections on port:%s\n", argv[ 2 ] );

    int aux = 5002;
    contador_partes = 0;

    for(int i = 0; i < 100; i++) {
        portnumaux_partes[i] = aux;
        aux++;
    }

    int aux2 = 4002;
    contador_principal = 0;
    contador = 0;
    bytesent = 0;

    for(int i = 0; i < 100; i++) {
        portnumaux_principal[i] = aux2;
        aux2++;
    }

    //pthread_mutex_init(&mutex_read, NULL);
    pthread_mutex_init(&mutex_size1, NULL);
    pthread_mutex_init(&mutex_size2, NULL);
    pthread_mutex_init(&mutex_size3, NULL);
    pthread_mutex_init(&mutex_size4, NULL);
    pthread_mutex_init(&mutex_size5, NULL);
    pthread_mutex_init(&mutex_size6, NULL);
    pthread_mutex_init(&mutex_size7, NULL);

    int fileqtd = 0;
    int nfiles = 0;
    int filessockets[100];
    int partssockets[100][100];


    while(1)
    {
        int communicationsocket = accept(mysocket, (struct sockaddr *)&dest, &socksize);
        argumento argumentos;

        if(readInt(communicationsocket) == 1) {
            fileqtd = readInt(communicationsocket);
            nfiles = readInt(communicationsocket);
            argumentos.fileqtd = fileqtd;
            argumentos.nfiles = nfiles;
            sendInt(contador, communicationsocket);

            for(int i = 0; i < fileqtd; i++) {
                struct sockaddr_in dest_s;
                struct sockaddr_in serv_s;
                int mysocket_s;
                socklen_t socksize_s = sizeof(struct sockaddr_in);

                memset(&serv_s, 0, sizeof(serv_s));           // zero the struct before filling the fields //
                serv_s.sin_family = AF_INET;                // set the type of connection to TCP/IP //
                serv_s.sin_addr.s_addr = htonl(INADDR_ANY); // set our address to any interface //
                serv_s.sin_port = htons( ((i+7+contador)*1000) ); 

                mysocket_s = socket(AF_INET, SOCK_STREAM, 0);
                bind(mysocket_s, (struct sockaddr *)&serv_s, sizeof(struct sockaddr));

                listen(mysocket_s, 10);

                //printf("server.c: %d\n", ((i+7+contador)*1000));

                //pthread_mutex_lock(&mutex_size5);
                argumentos.filessockets[i] = accept(mysocket_s, (struct sockaddr *)&dest_s, &socksize_s);
            }
            for(int i = 0; i < fileqtd; i++) {
                for(int j = 0; j < nfiles; j++) {
                    struct sockaddr_in dest_s2;
                    struct sockaddr_in serv_s2;
                    int mysocket_s2;
                    socklen_t socksize_s2 = sizeof(struct sockaddr_in);

                    memset(&serv_s2, 0, sizeof(serv_s2));           /* zero the struct before filling the fields */
                    serv_s2.sin_family = AF_INET;                /* set the type of connection to TCP/IP */
                    serv_s2.sin_addr.s_addr = htonl(INADDR_ANY); /* set our address to any interface */
                    serv_s2.sin_port = htons( ((i+7+contador)*1000)+1+j );

                    mysocket_s2 = socket(AF_INET, SOCK_STREAM, 0);
                    bind(mysocket_s2, (struct sockaddr *)&serv_s2, sizeof(struct sockaddr));

                    listen(mysocket_s2, 100);
                    argumentos.partssockets[i][j] = accept(mysocket_s2, (struct sockaddr *)&dest_s2, &socksize_s2);
                }
            }

            contador = contador + fileqtd;
        }

        

        argumentos.init = init;
        argumentos.filesdirectory = (char*)calloc(strlen(argv[1])+1, sizeof(char));
        strcpy(argumentos.filesdirectory, argv[1]);
        argumentos.communicationsocket = communicationsocket;

        pthread_create(&thread[count_thread], NULL, processclientrequest, &argumentos);
        count_thread++;


    }
    close(mysocket);
    return EXIT_SUCCESS;
}

