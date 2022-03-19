#include <stdbool.h>

typedef struct queue_t QUEUE;
typedef struct queue_node_t QUEUE_NODE;





QUEUE* queue_create( void );
bool queue_is_empty( QUEUE* );
bool enqueue( QUEUE* , const char* );
char* dequeue( QUEUE* );
void queue_destroy(QUEUE *);
int sizeoffila(QUEUE* fila);

//long count;
//pthread_mutex_t mutex_read;
//pthread_mutex_t mutex_write;
pthread_mutex_t mutex_size1;
pthread_mutex_t mutex_size2;
pthread_mutex_t mutex_size3;
pthread_mutex_t mutex_size4;
pthread_mutex_t mutex_size5;
pthread_mutex_t mutex_size6;
pthread_mutex_t mutex_size7;
//int portnumaux_s;
//int portnumaux_c;

