#include "circular_buffer.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

bool circular_buffer_init(circular_buffer_t* cb, size_t capacity, bool overwrite) {
    if (!cb || capacity == 0) {
        return false;
    }
    
    cb->buffer = (uint8_t*)malloc(capacity);
    if (!cb->buffer) {
        return false;
    }
    
    cb->capacity = capacity;
    cb->head = 0;
    cb->tail = 0;
    cb->count = 0;
    cb->overwrite = overwrite;
    
    return true;
}

void circular_buffer_destroy(circular_buffer_t* cb) {
    if (cb && cb->buffer) {
        free(cb->buffer);
        cb->buffer = NULL;
        cb->capacity = 0;
        cb->head = 0;
        cb->tail = 0;
        cb->count = 0;
    }
}

void circular_buffer_clear(circular_buffer_t* cb) {
    if (cb) {
        cb->head = 0;
        cb->tail = 0;
        cb->count = 0;
    }
}

bool circular_buffer_is_empty(const circular_buffer_t* cb) {
    return cb ? (cb->count == 0) : true;
}

bool circular_buffer_is_full(const circular_buffer_t* cb) {
    return cb ? (cb->count == cb->capacity) : true;
}

size_t circular_buffer_size(const circular_buffer_t* cb) {
    return cb ? cb->count : 0;
}

size_t circular_buffer_free(const circular_buffer_t* cb) {
    return cb ? (cb->capacity - cb->count) : 0;
}

size_t circular_buffer_write(circular_buffer_t* cb, const uint8_t* data, size_t size) {
    if (!cb || !data || size == 0) {
        return 0;
    }
    
    // If buffer is full and overwrite is disabled, return 0
    if (circular_buffer_is_full(cb) && !cb->overwrite) {
        return 0;
    }
    
    size_t written = 0;
    
    while (written < size) {
        // Calculate available space from tail to end of buffer
        size_t available_to_end = cb->capacity - cb->tail;
        size_t contiguous_write = (size - written < available_to_end) ? 
                                  (size - written) : available_to_end;
        
        // If buffer would overflow, handle overwrite
        if (cb->count + contiguous_write > cb->capacity) {
            if (!cb->overwrite) {
                break; // No overwrite allowed
            }
            
            // Need to advance head to make space
            size_t overflow = cb->count + contiguous_write - cb->capacity;
            cb->head = (cb->head + overflow) % cb->capacity;
            cb->count -= overflow;
        }
        
        // Copy data
        memcpy(&cb->buffer[cb->tail], &data[written], contiguous_write);
        written += contiguous_write;
        
        // Update tail and count
        cb->tail = (cb->tail + contiguous_write) % cb->capacity;
        cb->count += contiguous_write;
    }
    
    return written;
}

size_t circular_buffer_read(circular_buffer_t* cb, uint8_t* data, size_t size) {
    if (!cb || !data || size == 0 || circular_buffer_is_empty(cb)) {
        return 0;
    }
    
    size_t to_read = (size < cb->count) ? size : cb->count;
    size_t read = 0;
    
    while (read < to_read) {
        // Calculate available data from head to end of buffer
        size_t available_to_end = cb->capacity - cb->head;
        size_t contiguous_read = (to_read - read < available_to_end) ? 
                                 (to_read - read) : available_to_end;
        
        // Copy data
        memcpy(&data[read], &cb->buffer[cb->head], contiguous_read);
        read += contiguous_read;
        
        // Update head and count
        cb->head = (cb->head + contiguous_read) % cb->capacity;
        cb->count -= contiguous_read;
    }
    
    return read;
}

size_t circular_buffer_peek(const circular_buffer_t* cb, uint8_t* data, size_t size, size_t offset) {
    if (!cb || !data || size == 0 || offset >= cb->count) {
        return 0;
    }
    
    size_t to_peek = (size < cb->count - offset) ? size : (cb->count - offset);
    size_t peeked = 0;
    size_t virtual_head = (cb->head + offset) % cb->capacity;
    
    while (peeked < to_peek) {
        size_t available_to_end = cb->capacity - virtual_head;
        size_t contiguous_peek = (to_peek - peeked < available_to_end) ? 
                                 (to_peek - peeked) : available_to_end;
        
        memcpy(&data[peeked], &cb->buffer[virtual_head], contiguous_peek);
        peeked += contiguous_peek;
        virtual_head = (virtual_head + contiguous_peek) % cb->capacity;
    }
    
    return peeked;
}

const uint8_t* circular_buffer_get_read_ptr(const circular_buffer_t* cb, size_t* available) {
    if (!cb || circular_buffer_is_empty(cb)) {
        if (available) *available = 0;
        return NULL;
    }
    
    size_t contiguous_available = cb->capacity - cb->head;
    if (contiguous_available > cb->count) {
        contiguous_available = cb->count;
    }
    
    if (available) *available = contiguous_available;
    return &cb->buffer[cb->head];
}

void circular_buffer_advance_read(circular_buffer_t* cb, size_t size) {
    if (!cb || size == 0) {
        return;
    }
    
    size_t to_advance = (size < cb->count) ? size : cb->count;
    cb->head = (cb->head + to_advance) % cb->capacity;
    cb->count -= to_advance;
}

uint8_t* circular_buffer_get_write_ptr(circular_buffer_t* cb, size_t* available) {
    if (!cb || circular_buffer_is_full(cb)) {
        if (available) *available = 0;
        return NULL;
    }
    
    size_t contiguous_available = cb->capacity - cb->tail;
    size_t free_space = cb->capacity - cb->count;
    if (contiguous_available > free_space) {
        contiguous_available = free_space;
    }
    
    if (available) *available = contiguous_available;
    return &cb->buffer[cb->tail];
}

void circular_buffer_advance_write(circular_buffer_t* cb, size_t size) {
    if (!cb || size == 0) {
        return;
    }
    
    size_t free_space = cb->capacity - cb->count;
    size_t to_advance = (size < free_space) ? size : free_space;
    
    cb->tail = (cb->tail + to_advance) % cb->capacity;
    cb->count += to_advance;
}

ssize_t circular_buffer_find(const circular_buffer_t* cb, const uint8_t* pattern, size_t pattern_size) {
    if (!cb || !pattern || pattern_size == 0 || pattern_size > cb->count) {
        return -1;
    }
    
    for (size_t i = 0; i <= cb->count - pattern_size; i++) {
        bool found = true;
        
        for (size_t j = 0; j < pattern_size; j++) {
            size_t index = (cb->head + i + j) % cb->capacity;
            if (cb->buffer[index] != pattern[j]) {
                found = false;
                break;
            }
        }
        
        if (found) {
            return (ssize_t)i;
        }
    }
    
    return -1;
}

size_t circular_buffer_copy(circular_buffer_t* dest, const circular_buffer_t* src, size_t size) {
    if (!dest || !src || size == 0) {
        return 0;
    }
    
    size_t to_copy = (size < src->count) ? size : src->count;
    size_t free_space = dest->capacity - dest->count;
    
    if (to_copy > free_space && !dest->overwrite) {
        to_copy = free_space;
    }
    
    size_t copied = 0;
    size_t src_pos = src->head;
    
    while (copied < to_copy) {
        // Calculate contiguous block in source
        size_t src_available_to_end = src->capacity - src_pos;
        size_t src_contiguous = (to_copy - copied < src_available_to_end) ? 
                                (to_copy - copied) : src_available_to_end;
        
        // Write to destination
        size_t written = circular_buffer_write(dest, &src->buffer[src_pos], src_contiguous);
        if (written == 0) {
            break;
        }
        
        copied += written;
        src_pos = (src_pos + written) % src->capacity;
    }
    
    return copied;
}