#include "virt_board.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#include <termios.h>
#endif

// Global board instance
static virt_board_t board;
static virt_gpio_t gpio_pins[128];
static virt_interrupt_t interrupts[32];

// Performance counters
typedef struct {
    char name[32];
    uint32_t start_time;
    uint32_t total_time;
    uint32_t call_count;
} perf_counter_t;

static perf_counter_t perf_counters[16];
static uint32_t perf_counter_count = 0;

// Memory simulation
static uint8_t* ram_memory = NULL;
static uint8_t* flash_memory = NULL;

// Sleep function for simulation
#ifdef _WIN32
static void sleep_ms(uint32_t ms) { Sleep(ms); }
#else
static void sleep_ms(uint32_t ms) { usleep(ms * 1000); }
#endif

void virt_board_init(const virt_board_config_t* config) {
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║             VIRTUAL EMBEDDED BOARD                   ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    
    if (config) {
        memcpy(&board.config, config, sizeof(virt_board_config_t));
    } else {
        // Default configuration
        board.config.cpu_frequency_hz = 100000000;  // 100 MHz
        board.config.ram_size_kb = 256;             // 256 KB RAM
        board.config.flash_size_kb = 1024;          // 1 MB Flash
        board.config.system_clock_hz = 100000000;   // 100 MHz system clock
        board.config.debug_enabled = true;
        board.config.performance_counters = true;
    }
    
    // Initialize board state
    board.uptime_ms = 0;
    board.tick_counter = 0;
    board.instruction_count = 0;
    board.cycle_count = 0;
    board.running = true;
    board.reset_requested = false;
    board.sleep_mode = false;
    
    // Allocate memory
    size_t ram_size = board.config.ram_size_kb * 1024;
    size_t flash_size = board.config.flash_size_kb * 1024;
    
    ram_memory = (uint8_t*)malloc(ram_size);
    flash_memory = (uint8_t*)malloc(flash_size);
    
    if (!ram_memory || !flash_memory) {
        printf("[VIRT_BOARD] Error: Memory allocation failed\n");
        return;
    }
    
    // Initialize memory
    memset(ram_memory, 0x00, ram_size);      // RAM starts as 0x00
    memset(flash_memory, 0xFF, flash_size);  // Flash starts as 0xFF (erased)
    
    // Initialize GPIO pins
    memset(gpio_pins, 0, sizeof(gpio_pins));
    
    // Initialize interrupts
    memset(interrupts, 0, sizeof(interrupts));
    
    // Initialize performance counters
    memset(perf_counters, 0, sizeof(perf_counters));
    perf_counter_count = 0;
    
    printf("║ CPU:           ARM Cortex-M4 @ %u MHz             ║\n", 
           board.config.cpu_frequency_hz / 1000000);
    printf("║ RAM:           %u KB                              ║\n", 
           board.config.ram_size_kb);
    printf("║ Flash:         %u KB                              ║\n", 
           board.config.flash_size_kb);
    printf("║ System Clock:  %u MHz                            ║\n", 
           board.config.system_clock_hz / 1000000);
    printf("║ Debug:         %s                               ║\n", 
           board.config.debug_enabled ? "Enabled" : "Disabled");
    printf("║ Performance:   %s                               ║\n", 
           board.config.performance_counters ? "Enabled" : "Disabled");
    printf("╚══════════════════════════════════════════════════════╝\n");
    printf("\n[VIRT_BOARD] Virtual board initialized successfully\n");
}

void virt_board_deinit(void) {
    printf("[VIRT_BOARD] Deinitializing virtual board...\n");
    
    board.running = false;
    
    if (ram_memory) {
        free(ram_memory);
        ram_memory = NULL;
    }
    
    if (flash_memory) {
        free(flash_memory);
        flash_memory = NULL;
    }
    
    printf("[VIRT_BOARD] Virtual board deinitialized\n");
}

void virt_board_reset(void) {
    printf("[VIRT_BOARD] Resetting board...\n");
    
    board.uptime_ms = 0;
    board.tick_counter = 0;
    board.instruction_count = 0;
    board.cycle_count = 0;
    board.reset_requested = false;
    board.sleep_mode = false;
    
    // Reset GPIO pins
    memset(gpio_pins, 0, sizeof(gpio_pins));
    
    // Reset interrupts
    memset(interrupts, 0, sizeof(interrupts));
    
    printf("[VIRT_BOARD] Board reset complete\n");
}

void virt_board_run(void) {
    printf("[VIRT_BOARD] Starting board execution...\n");
    board.running = true;
}

void virt_board_stop(void) {
    printf("[VIRT_BOARD] Stopping board execution...\n");
    board.running = false;
}

void virt_board_tick(void) {
    if (!board.running || board.sleep_mode) {
        return;
    }
    
    board.tick_counter++;
    board.uptime_ms++;
    board.instruction_count += 100;  // Simulate 100 instructions per tick
    board.cycle_count += 100;        // Simulate 100 cycles per tick
    
    // Process interrupts
    for (int i = 0; i < 32; i++) {
        if (interrupts[i].enabled && interrupts[i].pending) {
            if (interrupts[i].handler) {
                printf("[VIRT_BOARD] Handling interrupt %u\n", i);
                interrupts[i].handler();
                interrupts[i].pending = false;
            }
        }
    }
    
    // Simulate tick time
    sleep_ms(1);
}

// GPIO functions
void virt_gpio_init(virt_gpio_t* gpio, uint8_t port, uint8_t pin) {
    if (!gpio || pin >= 128) return;
    
    gpio->port = port;
    gpio->pin = pin;
    gpio->value = false;
    gpio->direction = false;  // Input by default
    gpio->pull = true;        // Pull-up by default
    
    gpio_pins[pin] = *gpio;
    
    if (board.config.debug_enabled) {
        printf("[VIRT_GPIO] Initialized P%c%d\n", 'A' + port, pin);
    }
}

void virt_gpio_set_direction(virt_gpio_t* gpio, bool output) {
    if (!gpio) return;
    
    gpio->direction = output;
    gpio_pins[gpio->pin].direction = output;
    
    if (board.config.debug_enabled) {
        printf("[VIRT_GPIO] P%c%d direction: %s\n", 
               'A' + gpio->port, gpio->pin, 
               output ? "OUTPUT" : "INPUT");
    }
}

void virt_gpio_write(virt_gpio_t* gpio, bool value) {
    if (!gpio || !gpio->direction) return;
    
    gpio->value = value;
    gpio_pins[gpio->pin].value = value;
    
    if (board.config.debug_enabled) {
        printf("[VIRT_GPIO] P%c%d = %s\n", 
               'A' + gpio->port, gpio->pin, 
               value ? "HIGH" : "LOW");
    }
}

bool virt_gpio_read(virt_gpio_t* gpio) {
    if (!gpio) return false;
    
    // Simulate reading with some noise
    static uint32_t read_counter = 0;
    read_counter++;
    
    if (gpio->direction) {
        return gpio->value;
    } else {
        // For inputs, simulate occasional changes
        bool simulated_value = gpio_pins[gpio->pin].value;
        if (read_counter % 1000 == 0) {
            simulated_value = !simulated_value;
            if (board.config.debug_enabled) {
                printf("[VIRT_GPIO] Simulated input change on P%c%d\n",
                       'A' + gpio->port, gpio->pin);
            }
        }
        return simulated_value;
    }
}

void virt_gpio_toggle(virt_gpio_t* gpio) {
    if (!gpio || !gpio->direction) return;
    
    gpio->value = !gpio->value;
    gpio_pins[gpio->pin].value = gpio->value;
    
    if (board.config.debug_enabled) {
        printf("[VIRT_GPIO] Toggled P%c%d to %s\n",
               'A' + gpio->port, gpio->pin,
               gpio->value ? "HIGH" : "LOW");
    }
}

// Interrupt functions
void virt_interrupt_init(virt_interrupt_t* irq, uint8_t irq_number, 
                         void (*handler)(void), uint32_t priority) {
    if (!irq || irq_number >= 32) return;
    
    irq->irq_number = irq_number;
    irq->enabled = false;
    irq->pending = false;
    irq->handler = handler;
    irq->priority = priority;
    
    interrupts[irq_number] = *irq;
    
    if (board.config.debug_enabled) {
        printf("[VIRT_INTERRUPT] Initialized IRQ %u (priority: %u)\n", 
               irq_number, priority);
    }
}

void virt_interrupt_enable(virt_interrupt_t* irq) {
    if (!irq) return;
    
    irq->enabled = true;
    interrupts[irq->irq_number].enabled = true;
    
    if (board.config.debug_enabled) {
        printf("[VIRT_INTERRUPT] Enabled IRQ %u\n", irq->irq_number);
    }
}

void virt_interrupt_disable(virt_interrupt_t* irq) {
    if (!irq) return;
    
    irq->enabled = false;
    interrupts[irq->irq_number].enabled = false;
    
    if (board.config.debug_enabled) {
        printf("[VIRT_INTERRUPT] Disabled IRQ %u\n", irq->irq_number);
    }
}

void virt_interrupt_trigger(virt_interrupt_t* irq) {
    if (!irq || !irq->enabled) return;
    
    irq->pending = true;
    interrupts[irq->irq_number].pending = true;
    
    if (board.config.debug_enabled) {
        printf("[VIRT_INTERRUPT] Triggered IRQ %u\n", irq->irq_number);
    }
}

void virt_interrupt_clear(virt_interrupt_t* irq) {
    if (!irq) return;
    
    irq->pending = false;
    interrupts[irq->irq_number].pending = false;
}

bool virt_interrupt_is_pending(virt_interrupt_t* irq) {
    return irq ? irq->pending : false;
}

// System functions
uint32_t virt_board_get_uptime_ms(void) {
    return board.uptime_ms;
}

uint32_t virt_board_get_tick_count(void) {
    return board.tick_counter;
}

uint32_t virt_board_get_instruction_count(void) {
    return board.instruction_count;
}

uint32_t virt_board_get_cycle_count(void) {
    return board.cycle_count;
}

float virt_board_get_cpu_load(void) {
    // Simulate CPU load based on activity
    static uint32_t last_instruction_count = 0;
    static uint32_t last_update_time = 0;
    
    uint32_t current_time = board.uptime_ms;
    if (current_time - last_update_time >= 1000) {
        uint32_t instructions_per_second = board.instruction_count - last_instruction_count;
        last_instruction_count = board.instruction_count;
        last_update_time = current_time;
        
        // Assuming 100 million instructions per second at full load
        float load = (float)instructions_per_second / 100000000.0f * 100.0f;
        return (load > 100.0f) ? 100.0f : load;
    }
    
    return 50.0f; // Default return
}

void virt_board_enter_sleep_mode(void) {
    printf("[VIRT_BOARD] Entering sleep mode\n");
    board.sleep_mode = true;
}

void virt_board_exit_sleep_mode(void) {
    printf("[VIRT_BOARD] Exiting sleep mode\n");
    board.sleep_mode = false;
}

// Performance monitoring
void virt_board_start_perf_counter(const char* name) {
    if (!board.config.performance_counters || !name) return;
    
    // Find existing counter or create new one
    int index = -1;
    for (uint32_t i = 0; i < perf_counter_count; i++) {
        if (strcmp(perf_counters[i].name, name) == 0) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        if (perf_counter_count >= 16) return;
        index = perf_counter_count++;
        strncpy(perf_counters[index].name, name, 31);
        perf_counters[index].name[31] = '\0';
        perf_counters[index].total_time = 0;
        perf_counters[index].call_count = 0;
    }
    
    perf_counters[index].start_time = board.uptime_ms;
}

void virt_board_stop_perf_counter(const char* name) {
    if (!board.config.performance_counters || !name) return;
    
    uint32_t current_time = board.uptime_ms;
    
    for (uint32_t i = 0; i < perf_counter_count; i++) {
        if (strcmp(perf_counters[i].name, name) == 0) {
            uint32_t elapsed = current_time - perf_counters[i].start_time;
            perf_counters[i].total_time += elapsed;
            perf_counters[i].call_count++;
            break;
        }
    }
}

void virt_board_print_perf_counters(void) {
    if (!board.config.performance_counters) return;
    
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║             PERFORMANCE COUNTERS                     ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    
    for (uint32_t i = 0; i < perf_counter_count; i++) {
        float avg_time = perf_counters[i].call_count > 0 ? 
                        (float)perf_counters[i].total_time / perf_counters[i].call_count : 0.0f;
        
        printf("║ %-30s %5u calls, %5u ms total, %5.1f ms avg ║\n",
               perf_counters[i].name,
               perf_counters[i].call_count,
               perf_counters[i].total_time,
               avg_time);
    }
    
    printf("╚══════════════════════════════════════════════════════╝\n");
}

// Debug functions
void virt_board_debug_log(const char* format, ...) {
    if (!board.config.debug_enabled) return;
    
    va_list args;
    va_start(args, format);
    printf("[DEBUG] ");
    vprintf(format, args);
    printf("\n");
    va_end(args);
}

void virt_board_dump_registers(void) {
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║             VIRTUAL CPU REGISTERS                    ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║ R0-R3:  0x00000000 0x00000000 0x00000000 0x00000000 ║\n");
    printf("║ R4-R7:  0x00000000 0x00000000 0x00000000 0x00000000 ║\n");
    printf("║ R8-R11: 0x00000000 0x00000000 0x00000000 0x00000000 ║\n");
    printf("║ R12:    0x00000000  SP:       0x20001000  LR:        ║\n");
    printf("║ PC:     0x08000000  PSR:      0x01000000            ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");
}

void virt_board_dump_memory(uint32_t address, uint32_t size) {
    printf("\nMemory dump from 0x%08X to 0x%08X (%u bytes):\n",
           address, address + size - 1, size);
    
    for (uint32_t i = 0; i < size; i += 16) {
        printf("  0x%08X: ", address + i);
        
        for (uint32_t j = 0; j < 16 && i + j < size; j++) {
            uint32_t mem_addr = address + i + j;
            uint8_t value = 0xFF;
            
            if (mem_addr < board.config.ram_size_kb * 1024) {
                value = ram_memory[mem_addr];
            } else if (mem_addr < board.config.ram_size_kb * 1024 + 
                      board.config.flash_size_kb * 1024) {
                value = flash_memory[mem_addr - board.config.ram_size_kb * 1024];
            }
            
            printf("%02X ", value);
            if (j == 7) printf(" ");
        }
        
        printf("\n");
    }
}

// Board status
bool virt_board_is_running(void) {
    return board.running;
}

void virt_board_print_status(void) {
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║             VIRTUAL BOARD STATUS                     ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║ Uptime:          %-32u ms ║\n", board.uptime_ms);
    printf("║ Tick Count:      %-32u ║\n", board.tick_counter);
    printf("║ Instructions:    %-32u ║\n", board.instruction_count);
    printf("║ Cycles:          %-32u ║\n", board.cycle_count);
    printf("║ Running:         %-32s ║\n", board.running ? "YES" : "NO");
    printf("║ Sleep Mode:      %-32s ║\n", board.sleep_mode ? "YES" : "NO");
    printf("║ CPU Load:        %-31.1f%% ║\n", virt_board_get_cpu_load());
    printf("╚══════════════════════════════════════════════════════╝\n");
}