//
//  tsqueue.h
//  compare-and-swap
//
//  Created by Bernardo Breder on 01/02/16.
//  Copyright © 2016 breder. All rights reserved.
//

#ifndef tsqueue_h
#define tsqueue_h

#include <stdio.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdatomic.h>

struct flqueue_node_t {
    void *value;
    struct flqueue_node_t* next;
};

struct flqueue_head_t {
    struct flqueue_node_t* node;
};

struct flqueue_t {
    struct flqueue_node_t* node_buffer;
    _Atomic struct flqueue_head_t head;
    _Atomic struct flqueue_head_t tail;
    _Atomic struct flqueue_head_t free;
    _Atomic size_t size;
};

static inline size_t flqueue_size(struct flqueue_t *queue)
{
    return atomic_load(&queue->size);
}

static inline void flqueue_t_free(struct flqueue_t *queue)
{
    free(queue->node_buffer);
}

int flqueue_init(struct flqueue_t* queue, size_t max_size);
int flqueue_push(struct flqueue_t* queue, void *value);
void* flqueue_pop(struct flqueue_t* queue);

#endif /* tsqueue_h */
