#include "queue.h"

queue_t* queue_create(uint32_t capacity, uint32_t element_size) {
    queue_t* queue = (queue_t*)malloc(sizeof(queue_t));
    if (!queue) {
        printf("[QUEUE] Error: Failed to allocate queue structure\n");
        return NULL;
    }
    
    queue->buffer = malloc(capacity * element_size);
    if (!queue->buffer) {
        printf("[QUEUE] Error: Failed to allocate buffer\n");
        free(queue);
        return NULL;
    }
    
    queue->capacity = capacity;
    queue->element_size = element_size;
    queue->head = 0;
    queue->tail = 0;
    queue->count = 0;
    
    printf("[QUEUE] Created queue (capacity: %u, element size: %u)\n", 
           capacity, element_size);
    
    return queue;
}

void queue_destroy(queue_t* queue) {
    if (queue) {
        free(queue->buffer);
        free(queue);
        printf("[QUEUE] Destroyed queue\n");
    }
}

bool queue_enqueue(queue_t* queue, const void* element) {
    if (!queue) {
        printf("[QUEUE] Error: Queue is NULL\n");
        return false;
    }
    
    if (queue_is_full(queue)) {
        printf("[QUEUE] Warning: Queue is full (capacity: %u)\n", queue->capacity);
        return false;
    }
    
    void* dest = (uint8_t*)queue->buffer + (queue->tail * queue->element_size);
    memcpy(dest, element, queue->element_size);
    
    queue->tail = (queue->tail + 1) % queue->capacity;
    queue->count++;
    
    return true;
}

bool queue_dequeue(queue_t* queue, void* element) {
    if (!queue) {
        printf("[QUEUE] Error: Queue is NULL\n");
        return false;
    }
    
    if (queue_is_empty(queue)) {
        printf("[QUEUE] Warning: Queue is empty\n");
        return false;
    }
    
    void* src = (uint8_t*)queue->buffer + (queue->head * queue->element_size);
    memcpy(element, src, queue->element_size);
    
    queue->head = (queue->head + 1) % queue->capacity;
    queue->count--;
    
    return true;
}

bool queue_peek(queue_t* queue, void* element) {
    if (!queue || queue_is_empty(queue)) {
        return false;
    }
    
    void* src = (uint8_t*)queue->buffer + (queue->head * queue->element_size);
    memcpy(element, src, queue->element_size);
    
    return true;
}

uint32_t queue_count(queue_t* queue) {
    return queue ? queue->count : 0;
}

bool queue_is_empty(queue_t* queue) {
    return queue ? (queue->count == 0) : true;
}

bool queue_is_full(queue_t* queue) {
    return queue ? (queue->count == queue->capacity) : true;
}

void queue_clear(queue_t* queue) {
    if (queue) {
        queue->head = 0;
        queue->tail = 0;
        queue->count = 0;
        printf("[QUEUE] Cleared queue\n");
    }
}