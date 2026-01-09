#include "hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#include <conio.h>
#else
#include <unistd.h>
#endif

// System tick counter
static uint32_t system_tick_ms = 0;
static uint64_t system_tick_us = 0;
static clock_t program_start_time;

#ifdef _WIN32
void sleep_ms(uint32_t ms) { Sleep(ms); }
void sleep_us(uint32_t us) { 
    LARGE_INTEGER frequency, start, end;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    
    double microseconds = (double)us;
    double seconds = microseconds / 1000000.0;
    double target_ticks = seconds * frequency.QuadPart;
    
    QueryPerformanceCounter(&end);
    while ((end.QuadPart - start.QuadPart) < target_ticks) {
        QueryPerformanceCounter(&end);
    }
}
#else
void sleep_ms(uint32_t ms) { usleep(ms * 1000); }
void sleep_us(uint32_t us) { usleep(us); }
#endif

void hal_init(void) {
    program_start_time = clock();
    system_tick_ms = 0;
    system_tick_us = 0;
    
    printf("[HAL] Hardware Abstraction Layer Initialized\n");
    printf("[HAL] Virtual MCU: ARM Cortex-M4 @ 100MHz (simulated)\n");
    printf("[HAL] Virtual RAM: 256KB, Flash: 1MB\n");
    printf("[HAL] Virtual Peripherals: GPIO, UART, ADC, TIMER, SPI\n");
}

void hal_deinit(void) {
    printf("[HAL] Hardware Abstraction Layer Deinitialized\n");
}

// GPIO Functions
void hal_gpio_init(gpio_t* gpio) {
    if (gpio) {
        gpio->value = false;
        gpio->initialized = true;
        printf("[HAL_GPIO] Initialized GPIO P%c%d (Mode: %d, Pull: %d)\n",
               'A' + gpio->port, gpio->pin, gpio->mode, gpio->pull);
    }
}

void hal_gpio_deinit(gpio_t* gpio) {
    if (gpio && gpio->initialized) {
        gpio->initialized = false;
        printf("[HAL_GPIO] Deinitialized GPIO P%c%d\n",
               'A' + gpio->port, gpio->pin);
    }
}

void hal_gpio_write(gpio_t* gpio, bool value) {
    if (gpio && gpio->initialized) {
        gpio->value = value;
        if (gpio->mode == GPIO_MODE_OUTPUT) {
            printf("[HAL_GPIO] P%c%d = %s\n",
                   'A' + gpio->port, gpio->pin, value ? "HIGH" : "LOW");
        }
    }
}

bool hal_gpio_read(gpio_t* gpio) {
    if (gpio && gpio->initialized) {
        // Simulate reading with some noise
        static uint32_t counter = 0;
        counter++;
        
        if (gpio->mode == GPIO_MODE_INPUT) {
            // Simulate button press every 500 reads
            bool simulated_value = gpio->value;
            if (counter % 500 == 0) {
                simulated_value = !simulated_value;
                printf("[HAL_GPIO] Simulated button press on P%c%d\n",
                       'A' + gpio->port, gpio->pin);
            }
            return simulated_value;
        }
        return gpio->value;
    }
    return false;
}

void hal_gpio_toggle(gpio_t* gpio) {
    if (gpio && gpio->initialized) {
        gpio->value = !gpio->value;
        printf("[HAL_GPIO] Toggled P%c%d to %s\n",
               'A' + gpio->port, gpio->pin, gpio->value ? "HIGH" : "LOW");
    }
}

// UART Functions
void hal_uart_init(uart_t* uart, uint32_t baudrate) {
    if (uart) {
        uart->baudrate = baudrate;
        uart->rx_head = 0;
        uart->rx_tail = 0;
        uart->initialized = true;
        memset(uart->rx_buffer, 0, sizeof(uart->rx_buffer));
        
        printf("[HAL_UART] Initialized UART%d @ %u baud\n", 
               uart->id, baudrate);
    }
}

void hal_uart_deinit(uart_t* uart) {
    if (uart && uart->initialized) {
        uart->initialized = false;
        printf("[HAL_UART] Deinitialized UART%d\n", uart->id);
    }
}

void hal_uart_send(uart_t* uart, const uint8_t* data, uint32_t len) {
    if (uart && uart->initialized && data && len > 0) {
        printf("[HAL_UART%d] TX [%u bytes]: ", uart->id, len);
        
        // Print hex and ASCII
        for (uint32_t i = 0; i < len && i < 32; i++) {
            if (i % 16 == 0 && i > 0) printf("\n                    ");
            printf("%02X ", data[i]);
        }
        
        if (len > 32) {
            printf("... (truncated)");
        }
        
        // Print ASCII representation for printable characters
        printf("\n                    ASCII: ");
        for (uint32_t i = 0; i < len && i < 32; i++) {
            if (data[i] >= 32 && data[i] <= 126) {
                printf("%c", data[i]);
            } else {
                printf(".");
            }
        }
        printf("\n");
    }
}

uint32_t hal_uart_receive(uart_t* uart, uint8_t* buffer, uint32_t len) {
    if (!uart || !uart->initialized || !buffer || len == 0) {
        return 0;
    }
    
    static uint32_t simulation_counter = 0;
    simulation_counter++;
    
    // Simulate receiving data occasionally
    if (simulation_counter % 100 == 0) {
        const char* test_messages[] = {
            "SENSOR_DATA",
            "OTA_UPDATE_AVAILABLE",
            "SYSTEM_STATUS_OK",
            "TEMP:25.5C",
            "HUM:60%"
        };
        
        const char* message = test_messages[simulation_counter % 5];
        uint32_t msg_len = (uint32_t)strlen(message);
        uint32_t copy_len = msg_len < len ? msg_len : len;
        
        memcpy(buffer, message, copy_len);
        
        printf("[HAL_UART%d] RX [%u bytes]: %.*s\n",
               uart->id, copy_len, copy_len, buffer);
        
        return copy_len;
    }
    
    return 0;
}

bool hal_uart_available(uart_t* uart) {
    if (!uart || !uart->initialized) return false;
    
    static uint32_t check_counter = 0;
    check_counter++;
    
    // Simulate data available every 50 checks
    return (check_counter % 50 == 0);
}

void hal_uart_flush(uart_t* uart) {
    if (uart && uart->initialized) {
        uart->rx_head = 0;
        uart->rx_tail = 0;
        printf("[HAL_UART%d] Flushed RX buffer\n", uart->id);
    }
}

// ADC Functions
void hal_adc_init(adc_t* adc, uint32_t resolution) {
    if (adc) {
        adc->resolution = resolution;
        adc->value = 0;
        adc->sample_count = 0;
        printf("[HAL_ADC] Initialized ADC channel %u (%u-bit)\n",
               adc->channel, resolution);
    }
}

uint32_t hal_adc_read(adc_t* adc) {
    if (!adc) return 0;
    
    adc->sample_count++;
    
    // Generate simulated ADC values with some drift and noise
    static uint32_t base_values[] = {2048, 1024, 3072, 512, 3584};
    uint32_t base_idx = adc->channel % 5;
    uint32_t base_value = base_values[base_idx];
    
    // Add some drift and noise
    uint32_t drift = (adc->sample_count / 100) % 100;
    uint32_t noise = rand() % 50;
    
    adc->value = base_value + drift + noise - 25;
    
    // Clamp to resolution
    uint32_t max_value = (1 << adc->resolution) - 1;
    if (adc->value > max_value) {
        adc->value = max_value;
    }
    
    if (adc->sample_count % 200 == 0) {
        printf("[HAL_ADC] Channel %u: %u (0x%04X) [Samples: %u]\n",
               adc->channel, adc->value, adc->value, adc->sample_count);
    }
    
    return adc->value;
}

float hal_adc_read_voltage(adc_t* adc, float vref) {
    uint32_t raw = hal_adc_read(adc);
    uint32_t max_value = (1 << adc->resolution) - 1;
    return (float)raw * vref / max_value;
}

// Timer Functions
void hal_timer_init(timer_t* timer, uint32_t prescaler, uint32_t period) {
    if (timer) {
        timer->prescaler = prescaler;
        timer->period = period;
        timer->counter = 0;
        timer->running = false;
        printf("[HAL_TIMER] Initialized Timer %u (PSC: %u, ARR: %u)\n",
               timer->id, prescaler, period);
    }
}

void hal_timer_start(timer_t* timer) {
    if (timer) {
        timer->running = true;
        timer->counter = 0;
        printf("[HAL_TIMER] Started Timer %u\n", timer->id);
    }
}

void hal_timer_stop(timer_t* timer) {
    if (timer) {
        timer->running = false;
        printf("[HAL_TIMER] Stopped Timer %u\n", timer->id);
    }
}

uint32_t hal_timer_get_count(timer_t* timer) {
    if (timer && timer->running) {
        timer->counter++;
        if (timer->counter > timer->period) {
            timer->counter = 0;
        }
        
        if (timer->counter % 1000 == 0) {
            printf("[HAL_TIMER] Timer %u count: %u/%u\n",
                   timer->id, timer->counter, timer->period);
        }
        
        return timer->counter;
    }
    return 0;
}

void hal_timer_reset(timer_t* timer) {
    if (timer) {
        timer->counter = 0;
        printf("[HAL_TIMER] Reset Timer %u\n", timer->id);
    }
}

// System Functions
void hal_delay_ms(uint32_t ms) {
    sleep_ms(ms);
    system_tick_ms += ms;
    system_tick_us += ms * 1000;
}

void hal_delay_us(uint32_t us) {
    sleep_us(us);
    system_tick_us += us;
    system_tick_ms += us / 1000;
}

uint32_t hal_get_tick_ms(void) {
    clock_t current_time = clock();
    uint32_t elapsed_ms = (uint32_t)((current_time - program_start_time) * 1000 / CLOCKS_PER_SEC);
    system_tick_ms = elapsed_ms;
    return system_tick_ms;
}

uint64_t hal_get_tick_us(void) {
    clock_t current_time = clock();
    uint64_t elapsed_us = (uint64_t)((current_time - program_start_time) * 1000000 / CLOCKS_PER_SEC);
    system_tick_us = elapsed_us;
    return system_tick_us;
}

void hal_reset(void) {
    printf("[HAL] Simulating system reset...\n");
    system_tick_ms = 0;
    system_tick_us = 0;
    program_start_time = clock();
    printf("[HAL] System reset complete\n");
}