
#ifndef _QUEUE_H_
#define _QUEUE_H_

#include <stdlib.h>

// the heads prev is the most recently added element and the last to come out
// the heads next is the next element
// implemented with a dummy node, the head node is has no payload
// the head node has no next pointer, then the queue is empty

struct queue_t {
    void *data;
    struct queue_t *next;
    struct queue_t *prev;
};

// init a new node and return
struct queue_t *queue_node_init();
// free a node and set it to null
void queue_node_destroy(struct queue_t **node);
// init a queue head
struct queue_t *queue_init();
// destroy a queue, freeing all elms
void queue_destroy(struct queue_t **head);
// check if the queue is empty
int queue_is_empty(struct queue_t **head);
// to add an item, set the heads prev to be the item
void queue_enqueue(struct queue_t **head, struct queue_t *item);
// to remove an item, unlink from the queue and return
struct queue_t *queue_dequeue(struct queue_t **head);

#endif
