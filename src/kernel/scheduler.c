#include "scheduler.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

scheduler_t scheduler;

static uint32_t sys_time_ms = 0;

#ifdef _WIN32
void sleep_ms(int ms) {
    Sleep(ms);
}
#else
void sleep_ms(int ms) {
    usleep(ms * 1000);
}
#endif

void scheduler_init(void) {
    memset(&scheduler, 0, sizeof(scheduler));
    sys_time_ms = 0;
    printf("[SCHEDULER] Initialized\n");
}

static task_t* find_highest_priority_task(void) {
    task_t* highest = NULL;
    
    for (uint32_t i = 0; i < scheduler.task_count; i++) {
        task_t* task = scheduler.tasks[i];
        
        if (task->state == TASK_READY) {
            if (highest == NULL || task->priority < highest->priority) {
                highest = task;
            }
        }
    }
    
    return highest;
}

void scheduler_tick(void) {
    scheduler.tick_count++;
    sys_time_ms++;
    
    for (uint32_t i = 0; i < scheduler.task_count; i++) {
        task_t* task = scheduler.tasks[i];
        
        if (task->period_ms > 0) {
            if ((sys_time_ms - task->last_run) >= task->period_ms) {
                task->state = TASK_READY;
            }
        }
    }
    
    task_t* next_task = find_highest_priority_task();
    
    if (next_task && next_task->state == TASK_READY) {
        next_task->state = TASK_RUNNING;
        next_task->last_run = sys_time_ms;
        next_task->run_count++;
        
        next_task->task_func(next_task->arg);
        
        uint32_t exec_time = sys_time_ms - next_task->last_run;
        if (exec_time > next_task->deadline_ms && next_task->deadline_ms > 0) {
            next_task->missed_deadlines++;
        }
        
        next_task->state = TASK_READY;
    } else {
        scheduler.idle_time++;
    }
}

bool scheduler_add_task(void (*func)(void*), void* arg, uint32_t priority, 
                        uint32_t period_ms, const char* name) {
    if (scheduler.task_count >= MAX_TASKS) {
        printf("[SCHEDULER] Error: Max tasks reached (%d)\n", MAX_TASKS);
        return false;
    }
    
    task_t* task = (task_t*)malloc(sizeof(task_t));
    if (!task) {
        printf("[SCHEDULER] Error: Memory allocation failed\n");
        return false;
    }
    
    task->task_func = func;
    task->arg = arg;
    task->priority = priority;
    task->period_ms = period_ms;
    task->deadline_ms = period_ms;
    task->last_run = 0;
    task->state = TASK_READY;
    task->run_count = 0;
    task->missed_deadlines = 0;
    
    if (name) {
        strncpy(task->name, name, TASK_NAME_LEN - 1);
        task->name[TASK_NAME_LEN - 1] = '\0';
    } else {
        snprintf(task->name, TASK_NAME_LEN, "Task%d", scheduler.task_count);
    }
    
    scheduler.tasks[scheduler.task_count++] = task;
    printf("[SCHEDULER] Task added: %s (Priority: %u, Period: %ums)\n", 
           task->name, task->priority, task->period_ms);
    return true;
}

void scheduler_delay(uint32_t ms) {
    uint32_t end_time = sys_time_ms + ms;
    while (sys_time_ms < end_time) {
        sleep_ms(1);
    }
}

void scheduler_yield(void) {
    scheduler_tick();
}

void scheduler_start(void) {
    scheduler.running = true;
    printf("[SCHEDULER] Started with %d tasks\n", scheduler.task_count);
    
    while (scheduler.running) {
        scheduler_tick();
        sleep_ms(1);
    }
}

void scheduler_task_stats(void) {
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║                 TASK STATISTICS                      ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║ %-18s %-10s %-8s %-10s ║\n", "Task Name", "Runs", "Missed", "State");
    printf("╠══════════════════════════════════════════════════════╣\n");
    
    for (uint32_t i = 0; i < scheduler.task_count; i++) {
        task_t* task = scheduler.tasks[i];
        
        const char* state_str = "UNKNOWN";
        switch (task->state) {
            case TASK_READY: state_str = "READY"; break;
            case TASK_RUNNING: state_str = "RUNNING"; break;
            case TASK_BLOCKED: state_str = "BLOCKED"; break;
            case TASK_SUSPENDED: state_str = "SUSPENDED"; break;
        }
        
        printf("║ %-18s %-10u %-8u %-10s ║\n",
               task->name, task->run_count, task->missed_deadlines, state_str);
    }
    
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║ Total ticks: %-36u ║\n", scheduler.tick_count);
    printf("║ CPU Usage:   %-36.1f%% ║\n", scheduler_get_cpu_usage());
    printf("║ Idle time:   %-36u ║\n", scheduler.idle_time);
    printf("╚══════════════════════════════════════════════════════╝\n");
}

uint32_t scheduler_get_tick_count(void) {
    return scheduler.tick_count;
}

float scheduler_get_cpu_usage(void) {
    if (scheduler.tick_count == 0) return 0.0f;
    return 100.0f * (1.0f - (float)scheduler.idle_time / scheduler.tick_count);
}