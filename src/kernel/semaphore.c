#include "semaphore.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

void semaphore_init(semaphore_t* sem, uint32_t initial_count, uint32_t max_count) {
    sem->count = initial_count;
    sem->max_count = max_count;
    printf("[SEMAPHORE] Initialized (count: %u, max: %u)\n", initial_count, max_count);
}

bool semaphore_take(semaphore_t* sem, uint32_t timeout_ms) {
    if (!sem) {
        printf("[SEMAPHORE] Error: Semaphore is NULL\n");
        return false;
    }
    
    if (sem->count > 0) {
        sem->count--;
        printf("[SEMAPHORE] Taken (count now: %u)\n", sem->count);
        return true;
    }
    
    if (timeout_ms > 0) {
#ifdef _WIN32
        Sleep(timeout_ms);
#else
        usleep(timeout_ms * 1000);
#endif
    }
    
    printf("[SEMAPHORE] Failed to take (count: %u)\n", sem->count);
    return false;
}

bool semaphore_give(semaphore_t* sem) {
    if (!sem) {
        printf("[SEMAPHORE] Error: Semaphore is NULL\n");
        return false;
    }
    
    if (sem->count < sem->max_count) {
        sem->count++;
        printf("[SEMAPHORE] Given (count now: %u)\n", sem->count);
        return true;
    }
    
    printf("[SEMAPHORE] Failed to give (max count reached: %u)\n", sem->max_count);
    return false;
}

uint32_t semaphore_get_count(semaphore_t* sem) {
    return sem ? sem->count : 0;
}