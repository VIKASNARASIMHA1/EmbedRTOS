#ifndef HAL_H
#define HAL_H

#include <stdint.h>
#include <stdbool.h>

// GPIO Configuration
typedef enum {
    GPIO_MODE_INPUT,
    GPIO_MODE_OUTPUT,
    GPIO_MODE_ANALOG
} gpio_mode_t;

typedef enum {
    GPIO_PULL_NONE,
    GPIO_PULL_UP,
    GPIO_PULL_DOWN
} gpio_pull_t;

typedef struct {
    uint8_t port;
    uint8_t pin;
    gpio_mode_t mode;
    gpio_pull_t pull;
    bool value;
    bool initialized;
} gpio_t;

// UART Configuration
typedef struct {
    uint32_t id;
    uint32_t baudrate;
    uint8_t rx_buffer[128];
    uint32_t rx_head;
    uint32_t rx_tail;
    bool initialized;
} uart_t;

// ADC Configuration
typedef struct {
    uint32_t channel;
    uint32_t resolution;
    uint32_t value;
    uint32_t sample_count;
} adc_t;

// SPI Configuration
typedef struct {
    uint32_t id;
    uint32_t speed_hz;
    uint8_t mode;
    bool initialized;
} spi_t;

// Timer Configuration
typedef struct {
    uint32_t id;
    uint32_t prescaler;
    uint32_t period;
    uint32_t counter;
    bool running;
} timer_t;

// HAL Functions
void hal_init(void);
void hal_deinit(void);

// GPIO Functions
void hal_gpio_init(gpio_t* gpio);
void hal_gpio_deinit(gpio_t* gpio);
void hal_gpio_write(gpio_t* gpio, bool value);
bool hal_gpio_read(gpio_t* gpio);
void hal_gpio_toggle(gpio_t* gpio);

// UART Functions
void hal_uart_init(uart_t* uart, uint32_t baudrate);
void hal_uart_deinit(uart_t* uart);
void hal_uart_send(uart_t* uart, const uint8_t* data, uint32_t len);
uint32_t hal_uart_receive(uart_t* uart, uint8_t* buffer, uint32_t len);
bool hal_uart_available(uart_t* uart);
void hal_uart_flush(uart_t* uart);

// ADC Functions
void hal_adc_init(adc_t* adc, uint32_t resolution);
uint32_t hal_adc_read(adc_t* adc);
float hal_adc_read_voltage(adc_t* adc, float vref);

// Timer Functions
void hal_timer_init(timer_t* timer, uint32_t prescaler, uint32_t period);
void hal_timer_start(timer_t* timer);
void hal_timer_stop(timer_t* timer);
uint32_t hal_timer_get_count(timer_t* timer);
void hal_timer_reset(timer_t* timer);

// System Functions
void hal_delay_ms(uint32_t ms);
void hal_delay_us(uint32_t us);
uint32_t hal_get_tick_ms(void);
uint64_t hal_get_tick_us(void);
void hal_reset(void);

#endif