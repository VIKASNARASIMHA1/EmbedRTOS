#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Clear screen function
void clear_screen(void) {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

// Display main dashboard
void display_dashboard(uint32_t uptime, float cpu_usage, float temperature, 
                      float humidity, float pressure, uint32_t tasks, 
                      uint32_t memory_used, uint32_t memory_total) {
    clear_screen();
    
    printf("╔══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                    EMBEDDED SENSOR HUB - LIVE DASHBOARD                     ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║ System Status:                                                              ║\n");
    printf("║   Uptime: %8u s   CPU: %5.1f%%   Tasks: %2u   Memory: %4u/%4u KB             ║\n",
           uptime, cpu_usage, tasks, memory_used/1024, memory_total/1024);
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║ Sensor Readings:                                                            ║\n");
    printf("║   Temperature: %6.2f °C     Humidity: %6.1f %%     Pressure: %7.2f hPa       ║\n",
           temperature, humidity, pressure);
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    
    // CPU usage bar
    printf("║ CPU Usage:                                                                  ║\n");
    printf("║   [");
    int bar_width = 50;
    int filled = (int)(cpu_usage * bar_width / 100.0f);
    for (int i = 0; i < bar_width; i++) {
        if (i < filled) {
            printf("█");
        } else {
            printf(" ");
        }
    }
    printf("] %5.1f%%                                           ║\n", cpu_usage);
    
    // Memory usage bar
    printf("║ Memory Usage:                                                               ║\n");
    printf("║   [");
    float mem_percent = (float)memory_used * 100.0f / memory_total;
    filled = (int)(mem_percent * bar_width / 100.0f);
    for (int i = 0; i < bar_width; i++) {
        if (i < filled) {
            printf("█");
        } else {
            printf(" ");
        }
    }
    printf("] %5.1f%%                                           ║\n", mem_percent);
    
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║ Tasks:                                                                      ║\n");
    printf("║   • Sensor Task    [■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■] 80%% ║\n");
    printf("║   • Comm Task      [■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■              ] 50%% ║\n");
    printf("║   • Monitor Task   [■■■■■■■■■■■■■■■■■■                                ] 25%% ║\n");
    printf("║   • Idle Task      [■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■] 80%% ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║ Commands: [s]tats  [d]ashboard  [u]pdate  [t]est  [r]eset  [q]uit           ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════════════╝\n");
}

// Display ASCII art banner
void display_banner(void) {
    printf("\n");
    printf("   _____                          _           _   _____ _                 \n");
    printf("  | ____|_ __ _ __ ___  _ __   __| | ___ _ __| |_|  ___| |__   ___  _ __  \n");
    printf("  |  _| | '__| '__/ _ \\| '__| / _` |/ _ \\ '__| __| |_  | '_ \\ / _ \\| '_ \\ \n");
    printf("  | |___| |  | | | (_) | |   | (_| |  __/ |  | |_|  _| | | | | (_) | |_) |\n");
    printf("  |_____|_|  |_|  \\___/|_|    \\__,_|\\___|_|   \\__|_|   |_| |_|\\___/| .__/ \n");
    printf("                                                                   |_|    \n");
    printf("                     Software Simulator v1.0.0                             \n");
    printf("                     Designed for 8GB RAM Systems                         \n\n");
}

// Display test progress
void display_test_progress(const char* test_name, int current, int total) {
    printf("\r[TEST] %-30s [", test_name);
    
    int bar_width = 30;
    int progress = (current * bar_width) / total;
    
    for (int i = 0; i < bar_width; i++) {
        if (i < progress) {
            printf("=");
        } else if (i == progress) {
            printf(">");
        } else {
            printf(" ");
        }
    }
    
    printf("] %3d%%", (current * 100) / total);
    fflush(stdout);
}

// Display system information
void display_system_info(void) {
    clear_screen();
    
    printf("╔══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                         SYSTEM INFORMATION                                  ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║ Project:          Embedded Sensor Hub Simulator                            ║\n");
    printf("║ Version:          1.0.0                                                    ║\n");
    printf("║ Author:           Embedded Developer Portfolio                             ║\n");
    printf("║ License:          MIT                                                      ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║ Features Demonstrated:                                                     ║\n");
    printf("║   • Custom RTOS Scheduler with Priority-based Task Management              ║\n");
    printf("║   • Hardware Abstraction Layer (HAL) with Virtual Peripherals             ║\n");
    printf("║   • Kalman Filter for Sensor Fusion and Noise Reduction                   ║\n");
    printf("║   • OTA (Over-the-Air) Update System with Rollback Protection             ║\n");
    printf("║   • Custom Communication Protocol with CRC Validation                     ║\n");
    printf("║   • Circular Buffer Implementation for Efficient Data Handling            ║\n");
    printf("║   • Comprehensive Logging System with Multiple Levels                     ║\n");
    printf("║   • Virtual Hardware Simulation (GPIO, UART, ADC, SPI, I2C, Timers)       ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║ System Requirements:                                                       ║\n");
    printf("║   • RAM:           8 GB (uses < 50 MB)                                    ║\n");
    printf("║   • Storage:       500 MB free space                                      ║\n");
    printf("║   • OS:            Windows 10/11, Linux, or macOS                         ║\n");
    printf("║   • Graphics:      Terminal/Console only (no GPU required)                ║\n");
    printf("║   • Compiler:      GCC/MinGW or any C11 compatible compiler               ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nPress any key to continue...");
#ifdef _WIN32
    _getch();
#else
    system("read -n 1");
#endif
}

// Display help
void display_help(void) {
    clear_screen();
    
    printf("╔══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                            COMMAND HELP                                     ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║ [s] or [S] - Show detailed task statistics                                 ║\n");
    printf("║ [d] or [D] - Display real-time dashboard                                   ║\n");
    printf("║ [u] or [U] - Simulate OTA update process                                   ║\n");
    printf("║ [t] or [T] - Run system self-tests                                         ║\n");
    printf("║ [r] or [R] - Reset the simulation                                          ║\n");
    printf("║ [i] or [I] - Show system information                                       ║\n");
    printf("║ [h] or [H] - Show this help screen                                         ║\n");
    printf("║ [q] or [Q] - Quit the simulation                                           ║\n");
    printf("║ [ESC]      - Emergency shutdown                                            ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║ Interactive Features:                                                      ║\n");
    printf("║   • Real-time sensor data visualization                                    ║\n");
    printf("║   • CPU and memory usage monitoring                                        ║\n");
    printf("║   • Task execution statistics                                              ║\n");
    printf("║   • OTA update simulation with progress tracking                           ║\n");
    printf("║   • System health checks and diagnostics                                   ║\n");
    printf("╚══════════════════════════════════════════════════════════════════════════════╝\n");
    
    printf("\nPress any key to continue...");
#ifdef _WIN32
    _getch();
#else
    system("read -n 1");
#endif
}

// Display OTA update simulation
void display_ota_update(int progress, const char* phase) {
    clear_screen();
    
    printf("╔══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                     OTA UPDATE SIMULATION                                    ║\n");
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║ Current Phase: %-60s ║\n", phase);
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    
    // Progress bar
    printf("║ Progress:                                                                    ║\n");
    printf("║   [");
    int bar_width = 60;
    int filled = (progress * bar_width) / 100;
    for (int i = 0; i < bar_width; i++) {
        if (i < filled) {
            printf("█");
        } else {
            printf(" ");
        }
    }
    printf("] %3d%%                                           ║\n", progress);
    
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    
    // Steps
    char* steps[] = {
        "1. Checking for updates",
        "2. Downloading firmware",
        "3. Validating signature",
        "4. Writing to flash",
        "5. Verifying update",
        "6. Preparing for restart"
    };
    
    int current_step = (progress * 6) / 100;
    
    for (int i = 0; i < 6; i++) {
        if (i < current_step) {
            printf("║   ✓ %-70s ║\n", steps[i]);
        } else if (i == current_step) {
            printf("║   → %-70s ║\n", steps[i]);
        } else {
            printf("║     %-70s ║\n", steps[i]);
        }
    }
    
    printf("╠══════════════════════════════════════════════════════════════════════════════╣\n");
    printf("║ Status: %-70s ║\n", 
           progress < 100 ? "Update in progress..." : "Update complete!");
    printf("╚══════════════════════════════════════════════════════════════════════════════╝\n");
    
    if (progress < 100) {
        printf("\nUpdating... Please wait.\n");
    } else {
        printf("\nOTA update simulation complete! System restart required.\n");
    }
}