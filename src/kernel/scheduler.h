#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <stdint.h>
#include <stdbool.h>

#define MAX_TASKS           8
#define TASK_NAME_LEN       20
#define IDLE_TASK_PRIORITY  255

typedef enum {
    TASK_READY,
    TASK_RUNNING,
    TASK_BLOCKED,
    TASK_SUSPENDED
} task_state_t;

typedef struct {
    void (*task_func)(void*);
    void* arg;
    uint32_t priority;
    uint32_t period_ms;
    uint32_t deadline_ms;
    uint32_t last_run;
    task_state_t state;
    char name[TASK_NAME_LEN];
    uint32_t run_count;
    uint32_t missed_deadlines;
} task_t;

typedef struct {
    task_t* tasks[MAX_TASKS];
    uint32_t task_count;
    uint32_t tick_count;
    uint32_t idle_time;
    bool running;
} scheduler_t;

extern scheduler_t scheduler;

void scheduler_init(void);
void scheduler_start(void);
void scheduler_tick(void);
bool scheduler_add_task(void (*func)(void*), void* arg, uint32_t priority, 
                        uint32_t period_ms, const char* name);
void scheduler_delay(uint32_t ms);
void scheduler_yield(void);
void scheduler_task_stats(void);
uint32_t scheduler_get_tick_count(void);
float scheduler_get_cpu_usage(void);

#endif