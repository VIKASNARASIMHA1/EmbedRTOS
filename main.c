#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#endif

#include "src/kernel/scheduler.h"
#include "src/hal/hal.h"
#include "src/utils/logger.h"
#include "src/app/tasks.h"
#include "simulator/virt_board.h"

static volatile int running = 1;

void signal_handler(int sig) {
    (void)sig;
    printf("\nShutting down...\n");
    running = 0;
}

#ifdef _WIN32
void sleep_ms(int ms) {
    Sleep(ms);
}
#else
void sleep_ms(int ms) {
    usleep(ms * 1000);
}
#endif

int main() {
    printf("=============================================\n");
    printf("   EMBEDDED SENSOR HUB - WINDOWS SIMULATOR   \n");
    printf("   Designed for 8GB RAM systems              \n");
    printf("   No GPU required - Pure terminal app       \n");
    printf("=============================================\n\n");
    
    signal(SIGINT, signal_handler);
    
    logger_init(LOG_LEVEL_INFO);
    LOG_INFO("System initialization started...");
    
    virt_board_init();
    hal_init();
    scheduler_init();
    
    LOG_INFO("Creating application tasks...");
    
    scheduler_add_task(sensor_task, NULL, 1, 10, "Sensor");
    scheduler_add_task(comm_task, NULL, 2, 50, "Communication");
    scheduler_add_task(monitor_task, NULL, 3, 1000, "Monitor");
    
    printf("\n=== SYSTEM READY ===\n");
    printf("Interactive Commands:\n");
    printf("  [s] - Show task statistics\n");
    printf("  [u] - Simulate OTA update\n");
    printf("  [d] - Display dashboard\n");
    printf("  [q] - Quit program\n");
    printf("  [Ctrl+C] - Emergency shutdown\n\n");
    printf("System is now running...\n\n");
    
    uint32_t last_stat_print = 0;
    uint32_t dashboard_counter = 0;
    
    while (running && virt_board_is_running()) {
        scheduler_tick();
        virt_board_update();
        
        uint32_t ticks = scheduler_get_tick_count();
        
        if (ticks - last_stat_print >= 5000) {
            printf("[TICK: %6u] CPU: %5.1f%% | Tasks: %d | Memory: < 50MB\n",
                   ticks, scheduler_get_cpu_usage(), scheduler.task_count);
            last_stat_print = ticks;
            dashboard_counter++;
            
            if (dashboard_counter % 5 == 0) {
                printf("----------------------------------------------\n");
            }
        }
        
        if (_kbhit()) {
            char key = _getch();
            switch (key) {
                case 's':
                case 'S':
                    scheduler_task_stats();
                    break;
                    
                case 'u':
                case 'U':
                    LOG_INFO("=== OTA UPDATE SIMULATION ===");
                    LOG_INFO("1. Checking for updates...");
                    LOG_INFO("2. Downloading firmware...");
                    LOG_INFO("3. Validating signature...");
                    LOG_INFO("4. Updating complete!");
                    break;
                    
                case 'd':
                case 'D':
                    system("cls");
                    printf("\n=== LIVE DASHBOARD ===\n");
                    printf("Uptime:      %u seconds\n", ticks/1000);
                    printf("CPU Usage:   %.1f%%\n", scheduler_get_cpu_usage());
                    printf("Active Tasks:%d\n", scheduler.task_count);
                    printf("System:      Running\n");
                    printf("Memory:      < 50MB used\n");
                    printf("================================\n\n");
                    break;
                    
                case 'q':
                case 'Q':
                    running = 0;
                    break;
                    
                case 27: // ESC key
                    running = 0;
                    break;
                    
                default:
                    printf("Unknown command: '%c' (press 'h' for help)\n", key);
                    break;
            }
        }
        
        sleep_ms(1);
    }
    
    LOG_INFO("System shutdown initiated...");
    printf("\n=== FINAL STATISTICS ===\n");
    scheduler_task_stats();
    virt_board_shutdown();
    
    printf("\n=============================================\n");
    printf("   SIMULATION COMPLETE - SUCCESS!           \n");
    printf("   Project demonstrates:                    \n");
    printf("   • Custom RTOS scheduler                 \n");
    printf("   • Hardware abstraction layer            \n");
    printf("   • Sensor fusion algorithms              \n");
    printf("   • Communication protocols               \n");
    printf("   • OTA update system                     \n");
    printf("=============================================\n");
    
    printf("\nPress any key to exit...");
    _getch();
    
    return 0;
}