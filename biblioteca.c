#include "biblioteca.h"
#include "fila.h"

void sendInt( int n, int socket )
{

    int num_bytessent = 0;
    while((num_bytessent += send(socket, &n + num_bytessent, sizeof( int ) - num_bytessent, 0)) != sizeof(int));
}

int readInt( int socket )
{
    int n;

    int num_bytesread = 0;
    while((num_bytesread += recv(socket, &n + num_bytesread, sizeof( int ) - num_bytesread, 0)) != sizeof(int));

    return n;
}



void sendLong( long n, int socket )
{
    int num_bytessent = 0;

    while((num_bytessent += send(socket, &n + num_bytessent, sizeof( long ) - num_bytessent, 0)) != sizeof(long));
    //send(socket, &n, sizeof( long ), 0);
}

long readLong( int socket )
{
    long n;
    int num_bytesread = 0;

    while((num_bytesread += recv(socket, &n + num_bytesread, sizeof( long ) - num_bytesread, 0)) != sizeof(long));
    //recv(socket, &n, sizeof( long ), 0);
    return n;
}

void sendStr( char *str, int socket )
{
    int tam = strlen(str);
    sendInt(sizeof(char) * tam, socket);

    int num_bytessent = 0;
    while((num_bytessent += send(socket, str + num_bytessent, (sizeof(char) * tam) - num_bytessent, 0)) != sizeof(char) * tam);
    //send(socket, str, sizeof(char) * tam, MSG_NOSIGNAL);
}

char *readStr( int socket )
{
    int len = readInt(socket);
    char *str = (char *) calloc(len+1, sizeof(char));
    int num_bytesread = 0;
    while((num_bytesread += recv(socket, str + num_bytesread, len - num_bytesread, 0)) != len);
    //recv(socket, str, len, 0);
    str[len] = '\0';
    return str;
}

void sendVoid( void *v, int numofbytes, int socket )
{
    sendInt(numofbytes, socket);
    //send(socket, v, numofbytes, 0);

    int num_bytessent = 0;
    while((num_bytessent += send(socket, v + num_bytessent, numofbytes - num_bytessent, 0)) != numofbytes);

}

void *readVoid( int socket )
{
    int numofbytes = readInt(socket);
    void *buffer = (void *) calloc(numofbytes, 1);
    //recv(socket, buffer, numofbytes, 0);

    int num_bytesread = 0;
    while((num_bytesread += recv(socket, buffer + num_bytesread, numofbytes - num_bytesread, 0)) != numofbytes);


    return(buffer);
}

void listFiles(const char* dirname, QUEUE *fila)
{
    DIR* dir = opendir(dirname);
    if(dir == NULL) {
        printf("Directory not found\n");
        return;
    }

    printf("Reading files in: %s\n", dirname);

    struct dirent* entity;
    entity = readdir(dir);
    while(entity != NULL) {
        if(entity->d_type != 4) {
            //printf("%s\n", entity->d_name);
            enqueue(fila, entity->d_name);
        }
        if(entity->d_type == DT_DIR && strcmp(entity->d_name, ".") != 0 && strcmp(entity->d_name, "..") != 0) {
            char path[100] = { 0 };
            strcat(path, dirname);
            strcat(path, "/");
            strcat(path, entity->d_name);
            listFiles(path, fila);
        }

        entity = readdir(dir);
    }


    closedir(dir);
}


void sendFileNames( QUEUE *fila, int socket )
{
    int fileqtd = sizeoffila(fila);

    sendInt(fileqtd, socket);

    for(char *c = dequeue(fila); c != NULL; c = dequeue(fila)) {
        sendStr(c, socket);
    }
}

void sendstats( time_t init, int filesent, long bytesent, char *filesdirectory, int socket )
{
    int sec = 0, min = 0, hour = 0;
    sec = difftime(time(NULL), init);

    while(sec >= 60) {
        sec = sec - 60;
        min++;
    }
    while(min >= 60) {
        min = min - 60;
        hour++;
    }

    sendInt(hour, socket);
    sendInt(min, socket);
    sendInt(sec, socket);

    QUEUE *fila = queue_create();

    listFiles(filesdirectory, fila);
    int fileqtd = sizeoffila(fila);

    sendInt(fileqtd, socket);
    sendInt(filesent, socket);
    sendLong(bytesent, socket);
}

void *sendfilepart( void *ptr )
{
    //pthread_mutex_init(&mutex_read, NULL);
    //pthread_mutex_init(&mutex_size1, NULL);

    arg_send args = *((arg_send*)ptr);

    char *filename = args.filename;
    char *filesdirectory = args.filesdirectory;
    int nfiles = args.nfiles;
    int findex = args.findex;
    int socket = args.socket;

    int path_size = strlen(filesdirectory) + strlen(filename) + 2;
    char *path = calloc(path_size, sizeof(char));

    if (path == NULL) {
        perror("biblioteca.c: sendfiles: calloc");
        //free(filename);
        //free(path);
        return (void*) 0;
    }

    snprintf(path, path_size, "%s/%s", filesdirectory, filename);
    //printf("%d\n", findex);

    //pthread_mutex_lock(&mutex_size1);
    long filesize = returnFileSize_s(path);
    //long filesize = 845;
    //pthread_mutex_unlock(&mutex_size1);

    int filedescriptor = open(path, O_RDONLY);

    if (filedescriptor == -1) {
        perror("biblioteca.c: sendfiles: open");
        //free(filename);
        //free(path);
        return (void*) 0;
    }

    sendStr(filename, socket);
    sendInt(findex, socket);

    //printf("%s\n", filename);

    void *buffer = calloc(MAX_BUFFERSIZE, 1);
    //int v = 0;
    int num_bytesread = 0;
    int status = 0;
    long offset;
    int aux = 0;
    filesize--;
    if(findex == 0)
        offset = findex * filesize/nfiles;
    else {
        offset = findex * (filesize/nfiles + 1);
    }

    //printf("%d\n", findex);
        
    long readcounter = 0;

    while(1) {
        //pthread_mutex_lock(&mutex_read);
        if(filesize <= 500) {
            num_bytesread = pread(filedescriptor, buffer, (filesize/nfiles + 1) * 1, offset);
            aux = offset;
            offset = offset + (filesize/nfiles + 1) * 1;
        }
        else {
            if((filesize/nfiles + 1) > 500) {
                if((filesize/nfiles + 1) - readcounter >= 500) {
                    num_bytesread = pread(filedescriptor, buffer, MAX_BUFFERSIZE * 1, offset);
                    aux = offset;
                    offset = offset + MAX_BUFFERSIZE * 1;
                }
                else {
                    num_bytesread = pread(filedescriptor, buffer, (filesize/nfiles + 1) - readcounter * 1, offset);
                    aux = offset;
                    offset = offset + (filesize/nfiles + 1) - readcounter * 1;
                }
            }
            else {
                num_bytesread = pread(filedescriptor, buffer, (filesize/nfiles + 1) * 1, offset);
                aux = offset;
                offset = offset + (filesize/nfiles + 1) * 1;
            }
        }
        //pthread_mutex_unlock(&mutex_read);

        readcounter = readcounter + num_bytesread;

        status = 1;
        sendInt(status, socket);
        sendInt(num_bytesread, socket);
        sendVoid(buffer, num_bytesread * 1, socket);

        if(num_bytesread == 0 || filesize <= 500) {
            status = 0;
            pthread_mutex_lock(&mutex_size2);
            sendInt(status, socket);
            pthread_mutex_unlock(&mutex_size2);
            free(buffer);
            break;
        }
        
    }
    printf("part %d of %s was sent.\n", findex+1, filename);
    close(filedescriptor);
    return (void*) 0;
}

void *readfilepart( void *ptr )
{
    arg_read args = *((arg_read*)ptr);
    char *dirdestiny = args.dirdestiny;
    int socket = args.socket;

    char *c = readStr(socket);
    int findex = readInt(socket);

    long num_byteswrite = 0;

    int path_size = strlen(dirdestiny) + strlen(c) + 5;
    char *path = calloc(path_size, sizeof(char));
    snprintf(path, path_size, "%s/%s%d", dirdestiny, c, findex);

    int filedescriptor = open(path, O_CREAT|O_WRONLY, 0666);

    if (filedescriptor == -1) {
        perror("biblioteca.c: readfilepart: open"); 
        return (void*) 0;
    }
    long filesize = 1;

    while(1) {
        if(readInt(socket) == 0) {
            break;
        }
        pthread_mutex_lock(&mutex_size4);
        int num_bytessent = readInt(socket);
        pthread_mutex_unlock(&mutex_size4);
        if(num_bytessent > 500) {
            num_bytessent = 500;
        }
        void *data = readVoid(socket);
        num_byteswrite = pwrite(filedescriptor, data, num_bytessent * 1, filesize-1);   //mudar o pwrite para que seu offset seja do indice da parte do arquivo do sendfile (fazendo com que consiga escrever fora de ordem)
        filesize = filesize + num_byteswrite;
        
    }

    close(filedescriptor);

    return (void*) 0;
}

long returnFileSize_s(char *filename)
{
    
    FILE *fp = fopen(filename, "rb");

    long size = 0;

    if(fp) {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp)+1;
        fclose(fp);
    }

    return size;
}

long returnFileSize_c(char *filename)
{
    FILE *fp = fopen(filename, "rb");

    long size = 0;

    if(fp) {
        fseek(fp, 0, SEEK_END);
        size = ftell(fp)+1;
        fclose(fp);
    }
    if(size > 891000) {
        printf("%s\n", filename);
        printf("passou\n");
    }

    return size;
}

void *multithread_s(void *ptr) {

    argumento_s argumentos = *((argumento_s*)ptr);

    int comsocket = argumentos.communicationsocket;
    int portnum = argumentos.portnum;
    int nfiles = argumentos.nfiles;
    char *filesdirectory = argumentos.filesdirectory;
    
    pthread_t threads_s[nfiles];
    char *c;
    arg_send args[nfiles];
    
    c = readStr(comsocket);
    for(int i = 0; i < nfiles; i++) {
        args[i].filesdirectory = (char*)calloc(strlen(filesdirectory)+1, sizeof(char));
        strcpy(args[i].filesdirectory, filesdirectory);
        args[i].nfiles = nfiles;
        args[i].socket = argumentos.partssockets[i];
        
        
        args[i].filename = (char*)calloc(strlen(c)+1, sizeof(char));
        strcpy(args[i].filename, c);
        args[i].findex = i;
    }

    for(int i = 0; i < nfiles; i++) {
        pthread_create(&threads_s[i], NULL, sendfilepart, &args[i]);
    }

    for(int i = 0; i < nfiles; i++) {
        pthread_join(threads_s[i], NULL);
    }
    return (void*) 0;
}


void *multithread_c(void *ptr) {

    argumentos_c argumentos = *((argumentos_c*)ptr);

    int communicationsocket_principal = argumentos.communicationsocket;
    int numport = argumentos.portnum;
    int nfiles = argumentos.nfiles;
    char *clientrepository = argumentos.clientrepository;
    char *filename = argumentos.filename;

    arg_read args[nfiles];
    pthread_t threads_c[nfiles];
    char* portnum = (char*)calloc(5, sizeof(char));

    for(int i = 0; i < nfiles; i++) {
        args[i].nfiles = nfiles;
        args[i].dirdestiny = (char*)calloc(strlen(clientrepository)+1, sizeof(char));
        strcpy(args[i].dirdestiny, clientrepository);
    }
    sendStr(filename, communicationsocket_principal);

    for(int i = 0; i < nfiles; i++) {
        sprintf(portnum, "%d", numport+1+i);
        
        args[i].socket = argumentos.partssockets[i];
    }
    
    for(int i = 0; i < nfiles; i++) {
        pthread_create(&threads_c[i], NULL, readfilepart, &args[i]);
    }

    for(int i = 0; i < nfiles; i++) {
        pthread_join(threads_c[i], NULL);
    }

    /*for(int i = 0; i < nfiles; i++) {
        close(args[i].socket);
    }*/

    int final_size = strlen(clientrepository) + strlen(filename) + 5;
    char *final = calloc(final_size, sizeof(char));
    snprintf(final, final_size, "%s/%s", clientrepository, filename);

    int fd2 = open(final, O_CREAT|O_WRONLY, 0666);
    //char aux;
    int num_bytesread = 0;
    int num_byteswrite = 0;
    //long offset = 1;
    long offset_c = 1;

    for(int i = 0; i < nfiles; i++) {
        int path_size = strlen(clientrepository) + strlen(filename) + 5;
        char *path = calloc(path_size, sizeof(char));
        snprintf(path, path_size, "%s/%s%d", clientrepository, filename, i);
        long filesize = returnFileSize_s(path);
        void *buffer = calloc(filesize, 1);
        long readcounter = 0;
        long offset = 0;

        //FILE *fp1 = fopen(path, "r");
        int fd1 = open(path, O_RDONLY);

        //while((aux = fgetc(fp1)) != EOF) {
        //    fputc(aux, fp2);
        //}

        while(1) {
            if(filesize < 500) {
                num_bytesread = pread(fd1, buffer, (filesize + 1) * 1, offset);
                offset = offset + (filesize + 1) * 1;
            }
            else {
                if((filesize + 1) > 500) {
                    if((filesize + 1) - readcounter >= 500) {
                        num_bytesread = pread(fd1, buffer, MAX_BUFFERSIZE * 1, offset);
                        offset = offset + MAX_BUFFERSIZE * 1;
                    }
                    else {
                        num_bytesread = pread(fd1, buffer, (filesize + 1) - readcounter * 1, offset);
                        offset = offset + (filesize + 1) - readcounter * 1;
                    }
                }
                else {
                    num_bytesread = pread(fd1, buffer, (filesize + 1) * 1, offset);
                    offset = offset + (filesize + 1) * 1;
                }
            }

            readcounter = readcounter + num_bytesread;

            num_byteswrite = pwrite(fd2, buffer, num_bytesread * 1, offset_c-1);   //mudar o pwrite para que seu offset seja do indice da parte do arquivo do sendfile (fazendo com que consiga escrever fora de ordem)
            offset_c = offset_c + num_byteswrite;
            
            if(num_bytesread == 0 || filesize <= 500) {
                free(buffer);
                break;
            }



        }


        close(fd1);
        remove(path);
    }
    close(fd2);
    return (void*) 0;
}
