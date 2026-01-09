#ifndef CIRCULAR_BUFFER_H
#define CIRCULAR_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

// Circular buffer structure
typedef struct {
    uint8_t* buffer;
    size_t capacity;
    size_t head;
    size_t tail;
    size_t count;
    bool overwrite;
} circular_buffer_t;

// Initialize a circular buffer
bool circular_buffer_init(circular_buffer_t* cb, size_t capacity, bool overwrite);

// Destroy a circular buffer
void circular_buffer_destroy(circular_buffer_t* cb);

// Clear the buffer
void circular_buffer_clear(circular_buffer_t* cb);

// Check if buffer is empty
bool circular_buffer_is_empty(const circular_buffer_t* cb);

// Check if buffer is full
bool circular_buffer_is_full(const circular_buffer_t* cb);

// Get number of elements in buffer
size_t circular_buffer_size(const circular_buffer_t* cb);

// Get free space in buffer
size_t circular_buffer_free(const circular_buffer_t* cb);

// Write data to buffer
size_t circular_buffer_write(circular_buffer_t* cb, const uint8_t* data, size_t size);

// Read data from buffer
size_t circular_buffer_read(circular_buffer_t* cb, uint8_t* data, size_t size);

// Peek at data without removing it
size_t circular_buffer_peek(const circular_buffer_t* cb, uint8_t* data, size_t size, size_t offset);

// Get pointer to contiguous read region
const uint8_t* circular_buffer_get_read_ptr(const circular_buffer_t* cb, size_t* available);

// Advance read pointer
void circular_buffer_advance_read(circular_buffer_t* cb, size_t size);

// Get pointer to contiguous write region
uint8_t* circular_buffer_get_write_ptr(circular_buffer_t* cb, size_t* available);

// Advance write pointer
void circular_buffer_advance_write(circular_buffer_t* cb, size_t size);

// Find a byte pattern in buffer
ssize_t circular_buffer_find(const circular_buffer_t* cb, const uint8_t* pattern, size_t pattern_size);

// Copy data from one buffer to another
size_t circular_buffer_copy(circular_buffer_t* dest, const circular_buffer_t* src, size_t size);

#endif