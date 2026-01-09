#ifndef QUEUE_H
#define QUEUE_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void* buffer;
    uint32_t capacity;
    uint32_t element_size;
    uint32_t head;
    uint32_t tail;
    uint32_t count;
} queue_t;

queue_t* queue_create(uint32_t capacity, uint32_t element_size);
void queue_destroy(queue_t* queue);
bool queue_enqueue(queue_t* queue, const void* element);
bool queue_dequeue(queue_t* queue, void* element);
bool queue_peek(queue_t* queue, void* element);
uint32_t queue_count(queue_t* queue);
bool queue_is_empty(queue_t* queue);
bool queue_is_full(queue_t* queue);
void queue_clear(queue_t* queue);

#endif