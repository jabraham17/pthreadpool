

#include "queue.h"

// init a new node and return
struct queue_t *queue_node_init() {
    struct queue_t *node = (struct queue_t *)malloc(sizeof(struct queue_t));
    node->next = NULL;
    node->prev = NULL;
    node->data = NULL;
    return node;
}

// free a node and set it to null
void queue_node_destroy(struct queue_t **node) {
    free(*node);
    *node = NULL;
}

// init a queue head
struct queue_t *queue_init() {
    // the head is a dummy
    struct queue_t *head = queue_node_init();
    head->next = head;
    head->prev = head;
    return head;
}

// destroy a queue, freeing all elms
void queue_destroy(struct queue_t **head) {
    struct queue_t *node = NULL;
    // while not empty, get more
    while((node = queue_dequeue(head)) != NULL) {
        queue_node_destroy(&node);
    }
    // all thats left is the head
    queue_node_destroy(head);
}

// check if the queue is empty
int queue_is_empty(struct queue_t **head) {
    return *head && *head == (*head)->next;
}

// to add an item, set the heads prev to be the item
void queue_enqueue(struct queue_t **head, struct queue_t *item) {
    if(*head) {
        struct queue_t *last = (*head)->prev;

        last->next = item;
        item->prev = last;

        item->next = *head;
        (*head)->prev = item;
    }
}
// to remove an item, unlink from the queue and return
struct queue_t *queue_dequeue(struct queue_t **head) {
    if(*head && *head != (*head)->next) {
        struct queue_t *toDel = (*head)->next;
        (*head)->next = toDel->next;
        toDel->next->prev = *head;
        return toDel;
    }
    return NULL;
}
