#ifndef VIRT_BOARD_H
#define VIRT_BOARD_H

#include <stdint.h>
#include <stdbool.h>

// Virtual board configuration
typedef struct {
    uint32_t cpu_frequency_hz;      // CPU frequency in Hz
    uint32_t ram_size_kb;           // RAM size in KB
    uint32_t flash_size_kb;         // Flash size in KB
    uint32_t system_clock_hz;       // System clock in Hz
    bool debug_enabled;             // Debug output enabled
    bool performance_counters;      // Performance counters enabled
} virt_board_config_t;

// Virtual board state
typedef struct {
    virt_board_config_t config;
    uint32_t uptime_ms;
    uint32_t tick_counter;
    uint32_t instruction_count;
    uint32_t cycle_count;
    bool running;
    bool reset_requested;
    bool sleep_mode;
} virt_board_t;

// Virtual GPIO pin
typedef struct {
    uint8_t port;
    uint8_t pin;
    bool value;
    bool direction;  // true = output, false = input
    bool pull;       // true = pull-up, false = pull-down
} virt_gpio_t;

// Virtual interrupt
typedef struct {
    uint8_t irq_number;
    bool enabled;
    bool pending;
    void (*handler)(void);
    uint32_t priority;
} virt_interrupt_t;

// Function prototypes
void virt_board_init(const virt_board_config_t* config);
void virt_board_deinit(void);
void virt_board_reset(void);
void virt_board_run(void);
void virt_board_stop(void);
void virt_board_tick(void);

// GPIO functions
void virt_gpio_init(virt_gpio_t* gpio, uint8_t port, uint8_t pin);
void virt_gpio_set_direction(virt_gpio_t* gpio, bool output);
void virt_gpio_write(virt_gpio_t* gpio, bool value);
bool virt_gpio_read(virt_gpio_t* gpio);
void virt_gpio_toggle(virt_gpio_t* gpio);

// Interrupt functions
void virt_interrupt_init(virt_interrupt_t* irq, uint8_t irq_number, 
                         void (*handler)(void), uint32_t priority);
void virt_interrupt_enable(virt_interrupt_t* irq);
void virt_interrupt_disable(virt_interrupt_t* irq);
void virt_interrupt_trigger(virt_interrupt_t* irq);
void virt_interrupt_clear(virt_interrupt_t* irq);
bool virt_interrupt_is_pending(virt_interrupt_t* irq);

// System functions
uint32_t virt_board_get_uptime_ms(void);
uint32_t virt_board_get_tick_count(void);
uint32_t virt_board_get_instruction_count(void);
uint32_t virt_board_get_cycle_count(void);
float virt_board_get_cpu_load(void);
void virt_board_enter_sleep_mode(void);
void virt_board_exit_sleep_mode(void);

// Performance monitoring
void virt_board_start_perf_counter(const char* name);
void virt_board_stop_perf_counter(const char* name);
void virt_board_print_perf_counters(void);

// Debug functions
void virt_board_debug_log(const char* format, ...);
void virt_board_dump_registers(void);
void virt_board_dump_memory(uint32_t address, uint32_t size);

// Board status
bool virt_board_is_running(void);
void virt_board_print_status(void);

#endif