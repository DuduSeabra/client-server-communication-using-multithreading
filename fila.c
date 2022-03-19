#include "biblioteca.h"
#include "fila.h"

struct queue_t {
    struct queue_node_t *head;
    struct queue_node_t *tail;
};

struct queue_node_t {
    struct queue_node_t *next;
    char * data;
};



typedef struct queue_t QUEUE;
typedef struct queue_node_t QUEUE_NODE;



inline static bool    _is_empty  (QUEUE *);
inline static bool    _enqueue   (QUEUE *, const char * );
inline static char *  _dequeue   (QUEUE *);



/******************************************************************************
 ***                           INTERNAL FUNCTIONS                           ***
 ******************************************************************************/



inline static bool
_is_empty(QUEUE *queue) {
    return queue->head == NULL ? true : false;
}



inline static bool
_enqueue(QUEUE *queue, const char * data) {
    QUEUE_NODE *new_node = (QUEUE_NODE *) calloc(1, sizeof(QUEUE_NODE));

    if (new_node == NULL) {
        perror("nts_queue: _enqueue: calloc");
        exit(EXIT_FAILURE);
    }

    new_node->data = calloc(strlen(data)+1, sizeof(char));

    if (new_node->data == NULL) {
        perror("nts_queue: _enqueue: calloc");
        free(new_node);
        return false;
    }

    memcpy(new_node->data, data, strlen(data)+1);

    if (queue->tail == NULL) {
        queue->head = new_node;
    } else {
        queue->tail->next = new_node;
    }

    queue->tail = new_node;

    return true;
}



inline static char * 
_dequeue(QUEUE *queue) {
    QUEUE_NODE *temp = queue->head;
    char * data = temp->data;

    queue->head = queue->head->next;

    if (queue->head == NULL) {
        queue->tail = NULL;
    }

    free(temp);

    return data;
}



/******************************************************************************
 ***                           EXTERNAL FUNCTIONS                           ***
 ******************************************************************************/



QUEUE *
queue_create(void) {
    QUEUE *new_queue = (QUEUE *) calloc(1, sizeof(QUEUE));
    
    if (new_queue == NULL) {
        perror("nts_queue: queue_create: calloc");
        return NULL;
    }

    return new_queue;
}



bool
queue_is_empty(QUEUE *queue) {
    if (queue == NULL) {
        fputs("nts_queue: queue_is_empty: NULL pointer given\n", stderr);
        exit(EXIT_FAILURE);
    }

    return  _is_empty(queue);
}



bool
enqueue(QUEUE *queue, const char * data)
{
    if (queue == NULL) {
        fputs("nts_queue: enqueue: NULL pointer given\n", stderr);
        exit(EXIT_FAILURE);
    }

    if (data == NULL) {
        return false;
    }

    return _enqueue(queue, data);
}



char * 
dequeue(QUEUE *queue)
{
    if (queue == NULL) {
        fputs("nts_queue: dequeue: NULL pointer given\n", stderr);
        exit(EXIT_FAILURE);
    }
    
    return _is_empty(queue) ? NULL : _dequeue(queue);
}



inline void
queue_destroy(QUEUE *queue) {
    if (queue != NULL) {
        for (QUEUE_NODE *curr = queue->head; curr != NULL; curr = curr->next) {
            QUEUE_NODE *prev = curr;
            curr = curr->next;
            free(prev->data);
            free(prev);
        }

        free(queue);
    }
}

int sizeoffila(QUEUE *fila)
{
    int fileqtd = 0;

    QUEUE_NODE *aux = fila->head;

    if(aux == NULL)
        return 0;

    while(aux != NULL) {
        aux = aux->next;
        fileqtd++;
    }

    return fileqtd;
}
