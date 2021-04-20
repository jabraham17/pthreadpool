
#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdlib.h>

// the heads prev is the most recently added element and the last to come out
// the heads next is the next element
// implemented with a dummy node, the head node is has no payload
// the head node has no next pointer, then the queue is empty

typedef struct queue_t {
    void *data;
    struct queue_t *next;
    struct queue_t *prev;
}queue_t;

// init a new node and return
queue_t *queue_node_init();
// free a node and set it to null
void queue_node_destroy(queue_t **node, void (*clean_data)(void*));
// init a queue head
queue_t *queue_init();
// destroy a queue, freeing all elms
void queue_destroy(queue_t **head, void (*clean_data)(void*));
// check if the queue is empty
int queue_is_empty(queue_t **head);
// to add an item, set the heads prev to be the item
void queue_enqueue(queue_t **head, queue_t *item);
// to remove an item, unlink from the queue and return
queue_t *queue_dequeue(queue_t **head);

#endif
