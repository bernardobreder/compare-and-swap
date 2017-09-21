//
//  tsqueue.c
//  compare-and-swap
//
//  Created by Bernardo Breder on 01/02/16.
//  Copyright © 2016 breder. All rights reserved.
//

#include "flqueue.h"

int flqueue_init(struct flqueue_t* queue, size_t max_size)
{
    struct flqueue_head_t null = {ATOMIC_VAR_INIT(NULL)};
    queue->head = null;
    queue->tail = null;
    queue->size = ATOMIC_VAR_INIT(0);
    queue->node_buffer = malloc(max_size * sizeof(struct flqueue_node_t));
    if (queue->node_buffer == NULL) return 1;
    size_t i;
    for (i = 0; i < max_size - 1; i++) {
        queue->node_buffer[i].next = queue->node_buffer + i + 1;
    }
    queue->node_buffer[max_size - 1].next = NULL;
    struct flqueue_head_t free = {ATOMIC_VAR_INIT(queue->node_buffer)};
    queue->free = free;
    return 0;
}

static struct flqueue_node_t* pop(_Atomic struct flqueue_head_t* head)
{
    struct flqueue_head_t next, orig = atomic_load(head);
    do {
        if (orig.node == NULL) return NULL;
        next.node = orig.node->next;
    } while (!atomic_compare_exchange_weak(head, &orig, next));
    return orig.node;
}

static void push(_Atomic struct flqueue_head_t* head, struct flqueue_node_t* node)
{
    struct flqueue_head_t next, orig = atomic_load(head);
    do {
        node->next = orig.node;
        next.node = node;
    } while (!atomic_compare_exchange_weak(head, &orig, next));
}

int flqueue_push(struct flqueue_t* queue, void *value)
{
    struct flqueue_node_t *node = pop(&queue->free);
    if (node == NULL) return 1;
    node->next = NULL;
    node->value = value;
    
    struct flqueue_head_t null = {ATOMIC_VAR_INIT(NULL)};
    struct flqueue_head_t next, orig = atomic_load(&queue->tail);
    do {
        next.node = node;
    } while (!atomic_compare_exchange_weak(&queue->tail, &orig, next));
    if (orig.node) orig.node->next = node;
    atomic_compare_exchange_weak(&queue->head, &null, next);
    
    atomic_fetch_add(&queue->size, 1);
    return 0;
}

void* flqueue_pop(struct flqueue_t* queue)
{
    struct flqueue_node_t *node = pop(&queue->head);
    if (node == NULL) return NULL;
    atomic_fetch_sub(&queue->size, 1);
    void *value = node->value;
    push(&queue->free, node);
    return value;
}
