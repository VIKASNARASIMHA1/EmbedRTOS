#include "tasks.h"
#include "../kernel/scheduler.h"
#include "../utils/logger.h"
#include "../ota/ota_manager.h"
#include <stdio.h>
#include <stdlib.h>

// Global system status
static system_status_t system_status;

// Memory tracking (simulated)
static uint32_t total_memory = 1024 * 256; // 256KB simulated
static uint32_t used_memory = 0;

// Update memory usage (simulated)
static void update_memory_usage(void) {
    // Simulate memory usage with some variation
    static uint32_t base_usage = 50 * 1024; // 50KB base
    static int32_t variation = 0;
    
    // Add some random variation
    variation += (rand() % 100) - 50;
    if (variation > 50 * 1024) variation = 50 * 1024;
    if (variation < -50 * 1024) variation = -50 * 1024;
    
    used_memory = base_usage + variation;
    if (used_memory > total_memory) used_memory = total_memory;
    if (used_memory < 10 * 1024) used_memory = 10 * 1024;
    
    system_status.memory_used = used_memory;
    system_status.memory_total = total_memory;
}

// Check system health
static void check_system_health(void) {
    static uint32_t last_check = 0;
    uint32_t ticks = scheduler_get_tick_count();
    
    if (ticks - last_check >= 5000) { // Check every 5 seconds
        // Check CPU usage
        float cpu_usage = scheduler_get_cpu_usage();
        system_status.cpu_usage = cpu_usage;
        
        if (cpu_usage > 90.0f) {
            LOG_WARN("High CPU usage: %.1f%%", cpu_usage);
        } else if (cpu_usage < 5.0f) {
            LOG_WARN("Low CPU usage: %.1f%%", cpu_usage);
        }
        
        // Check memory usage
        float mem_usage = (float)used_memory * 100.0f / total_memory;
        if (mem_usage > 80.0f) {
            LOG_WARN("High memory usage: %.1f%%", mem_usage);
        }
        
        // Check task count
        if (scheduler.task_count < 3) {
            LOG_WARN("Low task count: %u", scheduler.task_count);
        }
        
        last_check = ticks;
    }
}

// Print system status
static void print_system_status(void) {
    static uint32_t last_print = 0;
    uint32_t ticks = scheduler_get_tick_count();
    
    if (ticks - last_print >= 10000) { // Print every 10 seconds
        LOG_INFO("System Status:");
        LOG_INFO("  Uptime: %u seconds", system_status.uptime_seconds);
        LOG_INFO("  CPU Usage: %.1f%%", system_status.cpu_usage);
        LOG_INFO("  Tasks: %u", system_status.task_count);
        LOG_INFO("  Memory: %u/%u KB (%.1f%%)", 
                system_status.memory_used / 1024,
                system_status.memory_total / 1024,
                (float)system_status.memory_used * 100.0f / system_status.memory_total);
        
        // Print OTA status if available
        if (ota_manager_is_update_available()) {
            LOG_INFO("  OTA: Update available (restart required)");
        }
        
        last_print = ticks;
    }
}

// Monitor task function
void monitor_task(void* arg) {
    (void)arg;
    
    LOG_INFO("Monitor task starting...");
    
    // Initialize system status
    memset(&system_status, 0, sizeof(system_status));
    system_status.task_count = scheduler.task_count;
    system_status.memory_total = total_memory;
    
    uint32_t startup_time = scheduler_get_tick_count();
    
    while (1) {
        // Update system status
        system_status.uptime_seconds = (scheduler_get_tick_count() - startup_time) / 1000;
        system_status.cpu_usage = scheduler_get_cpu_usage();
        system_status.task_count = scheduler.task_count;
        
        // Update memory usage
        update_memory_usage();
        
        // Check system health
        check_system_health();
        
        // Print status periodically
        print_system_status();
        
        // Simulate monitoring overhead
        scheduler_delay(MONITOR_TASK_PERIOD_MS);
    }
}

// Getter for system status
system_status_t* get_system_status(void) {
    return &system_status;
}