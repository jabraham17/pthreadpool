

#include "queue.h"

// init a new node and return
queue_t *queue_node_init() {
    queue_t *node = (queue_t *)malloc(sizeof(queue_t));
    node->next = NULL;
    node->prev = NULL;
    node->data = NULL;
    return node;
}

// free a node and set it to null
void queue_node_destroy(queue_t **node, void (*clean_data)(void*)) {
    if(clean_data) clean_data((*node)->data);
    free(*node);
    *node = NULL;
}

// init a queue head
queue_t *queue_init() {
    // the head is a dummy
    queue_t *head = queue_node_init();
    head->next = head;
    head->prev = head;
    return head;
}

// destroy a queue, freeing all elms
void queue_destroy(queue_t **head, void (*clean_data)(void*)) {
    queue_t *node = NULL;
    // while not empty, get more
    while((node = queue_dequeue(head)) != NULL) {
        queue_node_destroy(&node, clean_data);
    }
    // all thats left is the head
    //no destructor needed, its empty
    queue_node_destroy(head, NULL);
}

// check if the queue is empty
int queue_is_empty(queue_t **head) {
    return *head && *head == (*head)->next;
}

// to add an item, set the heads prev to be the item
void queue_enqueue(queue_t **head, queue_t *item) {
    if(*head) {
        queue_t *last = (*head)->prev;

        last->next = item;
        item->prev = last;

        item->next = *head;
        (*head)->prev = item;
    }
}
// to remove an item, unlink from the queue and return
queue_t *queue_dequeue(queue_t **head) {
    if(*head && *head != (*head)->next) {
        queue_t *toDel = (*head)->next;
        (*head)->next = toDel->next;
        toDel->next->prev = *head;
        return toDel;
    }
    return NULL;
}
