#ifndef VIRT_PERIPH_H
#define VIRT_PERIPH_H

#include <stdint.h>
#include <stdbool.h>

// Virtual SPI Peripheral
typedef struct {
    uint32_t CR1;
    uint32_t CR2;
    uint32_t SR;
    uint32_t DR;
    uint32_t CRCPR;
    uint32_t RXCRCR;
    uint32_t TXCRCR;
    uint32_t I2SCFGR;
    uint32_t I2SPR;
    bool initialized;
} virt_spi_t;

// Virtual I2C Peripheral
typedef struct {
    uint32_t CR1;
    uint32_t CR2;
    uint32_t OAR1;
    uint32_t OAR2;
    uint32_t DR;
    uint32_t SR1;
    uint32_t SR2;
    uint32_t CCR;
    uint32_t TRISE;
    bool initialized;
} virt_i2c_t;

// Virtual DMA Peripheral
typedef struct {
    uint32_t ISR;
    uint32_t IFCR;
    uint32_t CCR[8];
    uint32_t CNDTR[8];
    uint32_t CPAR[8];
    uint32_t CMAR[8];
    bool initialized;
} virt_dma_t;

// Virtual RTC Peripheral
typedef struct {
    uint32_t TR;
    uint32_t DR;
    uint32_t CR;
    uint32_t ISR;
    uint32_t PRER;
    uint32_t WUTR;
    uint32_t CALIBR;
    uint32_t ALRMAR;
    uint32_t ALRMBR;
    uint32_t WPR;
    uint32_t SSR;
    uint32_t SHIFTR;
    uint32_t TSTR;
    uint32_t TSDR;
    uint32_t TSSSR;
    uint32_t CALR;
    uint32_t TAFCR;
    uint32_t ALRMASSR;
    uint32_t ALRMBSSR;
    uint32_t BKP[32];
    bool initialized;
} virt_rtc_t;

// Function Prototypes
void virt_spi_init(virt_spi_t* spi, uint32_t speed_hz, uint8_t mode);
void virt_spi_deinit(virt_spi_t* spi);
void virt_spi_transfer(virt_spi_t* spi, const uint8_t* tx_data, 
                       uint8_t* rx_data, uint32_t size);
bool virt_spi_is_busy(virt_spi_t* spi);

void virt_i2c_init(virt_i2c_t* i2c, uint32_t speed_hz);
void virt_i2c_deinit(virt_i2c_t* i2c);
bool virt_i2c_write(virt_i2c_t* i2c, uint8_t dev_addr, 
                    const uint8_t* data, uint32_t len);
bool virt_i2c_read(virt_i2c_t* i2c, uint8_t dev_addr, 
                   uint8_t* data, uint32_t len);
bool virt_i2c_is_busy(virt_i2c_t* i2c);

void virt_dma_init(virt_dma_t* dma);
void virt_dma_deinit(virt_dma_t* dma);
void virt_dma_config_channel(virt_dma_t* dma, uint8_t channel, 
                             uint32_t src_addr, uint32_t dst_addr, 
                             uint32_t size, uint32_t config);
void virt_dma_start(virt_dma_t* dma, uint8_t channel);
void virt_dma_stop(virt_dma_t* dma, uint8_t channel);
bool virt_dma_is_busy(virt_dma_t* dma, uint8_t channel);

void virt_rtc_init(virt_rtc_t* rtc);
void virt_rtc_deinit(virt_rtc_t* rtc);
void virt_rtc_set_time(virt_rtc_t* rtc, uint8_t hour, uint8_t minute, 
                       uint8_t second);
void virt_rtc_set_date(virt_rtc_t* rtc, uint8_t year, uint8_t month, 
                       uint8_t day, uint8_t weekday);
void virt_rtc_get_time(virt_rtc_t* rtc, uint8_t* hour, uint8_t* minute, 
                       uint8_t* second);
void virt_rtc_get_date(virt_rtc_t* rtc, uint8_t* year, uint8_t* month, 
                       uint8_t* day, uint8_t* weekday);
uint32_t virt_rtc_get_timestamp(virt_rtc_t* rtc);

#endif