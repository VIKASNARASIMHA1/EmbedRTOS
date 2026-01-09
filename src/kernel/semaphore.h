#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint32_t count;
    uint32_t max_count;
} semaphore_t;

void semaphore_init(semaphore_t* sem, uint32_t initial_count, uint32_t max_count);
bool semaphore_take(semaphore_t* sem, uint32_t timeout_ms);
bool semaphore_give(semaphore_t* sem);
uint32_t semaphore_get_count(semaphore_t* sem);

#endif