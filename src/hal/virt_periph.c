#include "virt_periph.h"
#include <stdio.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// ==================== SPI FUNCTIONS ====================
void virt_spi_init(virt_spi_t* spi, uint32_t speed_hz, uint8_t mode) {
    if (spi) {
        spi->CR1 = 0x0000;
        spi->CR2 = 0x0000;
        spi->SR = 0x0002; // TXE flag set
        spi->DR = 0x00;
        spi->initialized = true;
        
        printf("[VIRT_SPI] Initialized (Speed: %u Hz, Mode: %u)\n", 
               speed_hz, mode);
    }
}

void virt_spi_deinit(virt_spi_t* spi) {
    if (spi && spi->initialized) {
        spi->initialized = false;
        printf("[VIRT_SPI] Deinitialized\n");
    }
}

void virt_spi_transfer(virt_spi_t* spi, const uint8_t* tx_data, 
                       uint8_t* rx_data, uint32_t size) {
    if (!spi || !spi->initialized) {
        printf("[VIRT_SPI] Error: SPI not initialized\n");
        return;
    }
    
    printf("[VIRT_SPI] Transfer %u bytes:\n", size);
    printf("  TX: ");
    
    for (uint32_t i = 0; i < size && i < 16; i++) {
        printf("%02X ", tx_data ? tx_data[i] : 0xFF);
        if (rx_data) {
            // Echo back transmitted data for simulation
            rx_data[i] = tx_data ? tx_data[i] : 0x00;
            
            // Add some simulated noise
            if ((i % 5) == 0) {
                rx_data[i] ^= 0x01; // Flip LSB occasionally
            }
        }
    }
    
    if (size > 16) {
        printf("... (truncated)");
    }
    
    printf("\n");
    
    // Simulate transfer time
#ifdef _WIN32
    Sleep(1);
#else
    usleep(1000);
#endif
}

bool virt_spi_is_busy(virt_spi_t* spi) {
    if (!spi || !spi->initialized) return false;
    
    static uint32_t counter = 0;
    counter++;
    
    // Simulate busy every 10 calls
    return (counter % 10 == 0);
}

// ==================== I2C FUNCTIONS ====================
void virt_i2c_init(virt_i2c_t* i2c, uint32_t speed_hz) {
    if (i2c) {
        i2c->CR1 = 0x0000;
        i2c->CR2 = 0x0000;
        i2c->initialized = true;
        
        printf("[VIRT_I2C] Initialized (Speed: %u Hz)\n", speed_hz);
    }
}

void virt_i2c_deinit(virt_i2c_t* i2c) {
    if (i2c && i2c->initialized) {
        i2c->initialized = false;
        printf("[VIRT_I2C] Deinitialized\n");
    }
}

bool virt_i2c_write(virt_i2c_t* i2c, uint8_t dev_addr, 
                    const uint8_t* data, uint32_t len) {
    if (!i2c || !i2c->initialized || !data || len == 0) {
        printf("[VIRT_I2C] Error: Invalid parameters\n");
        return false;
    }
    
    printf("[VIRT_I2C] Write to device 0x%02X (%u bytes): ", dev_addr, len);
    
    for (uint32_t i = 0; i < len && i < 16; i++) {
        printf("%02X ", data[i]);
    }
    
    if (len > 16) {
        printf("... (truncated)");
    }
    
    printf("\n");
    
    // Simulate I2C transfer time
#ifdef _WIN32
    Sleep(2);
#else
    usleep(2000);
#endif
    
    return true;
}

bool virt_i2c_read(virt_i2c_t* i2c, uint8_t dev_addr, 
                   uint8_t* data, uint32_t len) {
    if (!i2c || !i2c->initialized || !data || len == 0) {
        printf("[VIRT_I2C] Error: Invalid parameters\n");
        return false;
    }
    
    // Generate simulated I2C data
    static uint8_t simulated_data = 0x00;
    
    for (uint32_t i = 0; i < len; i++) {
        data[i] = simulated_data++;
    }
    
    printf("[VIRT_I2C] Read from device 0x%02X (%u bytes): ", dev_addr, len);
    
    for (uint32_t i = 0; i < len && i < 16; i++) {
        printf("%02X ", data[i]);
    }
    
    if (len > 16) {
        printf("... (truncated)");
    }
    
    printf("\n");
    
    // Simulate I2C transfer time
#ifdef _WIN32
    Sleep(2);
#else
    usleep(2000);
#endif
    
    return true;
}

bool virt_i2c_is_busy(virt_i2c_t* i2c) {
    if (!i2c || !i2c->initialized) return false;
    
    static uint32_t counter = 0;
    counter++;
    
    // Simulate busy every 15 calls
    return (counter % 15 == 0);
}

// ==================== DMA FUNCTIONS ====================
void virt_dma_init(virt_dma_t* dma) {
    if (dma) {
        dma->ISR = 0x00000000;
        dma->initialized = true;
        
        for (int i = 0; i < 8; i++) {
            dma->CCR[i] = 0x00000000;
            dma->CNDTR[i] = 0x00000000;
            dma->CPAR[i] = 0x00000000;
            dma->CMAR[i] = 0x00000000;
        }
        
        printf("[VIRT_DMA] Initialized\n");
    }
}

void virt_dma_deinit(virt_dma_t* dma) {
    if (dma && dma->initialized) {
        dma->initialized = false;
        printf("[VIRT_DMA] Deinitialized\n");
    }
}

void virt_dma_config_channel(virt_dma_t* dma, uint8_t channel, 
                             uint32_t src_addr, uint32_t dst_addr, 
                             uint32_t size, uint32_t config) {
    if (!dma || !dma->initialized || channel >= 8) {
        printf("[VIRT_DMA] Error: Invalid channel or DMA not initialized\n");
        return;
    }
    
    dma->CCR[channel] = config;
    dma->CNDTR[channel] = size;
    dma->CPAR[channel] = src_addr;
    dma->CMAR[channel] = dst_addr;
    
    printf("[VIRT_DMA] Channel %u configured:\n", channel);
    printf("  Source: 0x%08X\n", src_addr);
    printf("  Dest:   0x%08X\n", dst_addr);
    printf("  Size:   %u bytes\n", size);
    printf("  Config: 0x%08X\n", config);
}

void virt_dma_start(virt_dma_t* dma, uint8_t channel) {
    if (!dma || !dma->initialized || channel >= 8) {
        printf("[VIRT_DMA] Error: Invalid channel or DMA not initialized\n");
        return;
    }
    
    dma->CCR[channel] |= 0x00000001; // Enable bit
    
    printf("[VIRT_DMA] Channel %u started (transferring %u bytes)\n",
           channel, dma->CNDTR[channel]);
    
    // Simulate DMA transfer completion
    dma->CNDTR[channel] = 0;
    dma->ISR |= (1 << (channel * 4)); // Set TCIF flag
    
#ifdef _WIN32
    Sleep(1);
#else
    usleep(1000);
#endif
    
    printf("[VIRT_DMA] Channel %u transfer complete\n", channel);
}

void virt_dma_stop(virt_dma_t* dma, uint8_t channel) {
    if (!dma || !dma->initialized || channel >= 8) {
        printf("[VIRT_DMA] Error: Invalid channel or DMA not initialized\n");
        return;
    }
    
    dma->CCR[channel] &= ~0x00000001; // Disable bit
    
    printf("[VIRT_DMA] Channel %u stopped\n", channel);
}

bool virt_dma_is_busy(virt_dma_t* dma, uint8_t channel) {
    if (!dma || !dma->initialized || channel >= 8) {
        return false;
    }
    
    return (dma->CCR[channel] & 0x00000001) != 0;
}

// ==================== RTC FUNCTIONS ====================
void virt_rtc_init(virt_rtc_t* rtc) {
    if (rtc) {
        memset(rtc, 0, sizeof(virt_rtc_t));
        rtc->initialized = true;
        
        // Set default time and date
        time_t now = time(NULL);
        struct tm* local_time = localtime(&now);
        
        rtc->TR = ((local_time->tm_hour / 10) << 20) |
                  ((local_time->tm_hour % 10) << 16) |
                  ((local_time->tm_min / 10) << 12) |
                  ((local_time->tm_min % 10) << 8) |
                  ((local_time->tm_sec / 10) << 4) |
                  (local_time->tm_sec % 10);
        
        rtc->DR = (((local_time->tm_year - 100) / 10) << 20) |
                  (((local_time->tm_year - 100) % 10) << 16) |
                  (((local_time->tm_mon + 1) / 10) << 12) |
                  (((local_time->tm_mon + 1) % 10) << 8) |
                  ((local_time->tm_mday / 10) << 4) |
                  (local_time->tm_mday % 10);
        
        printf("[VIRT_RTC] Initialized\n");
        printf("[VIRT_RTC] Current time: %02d:%02d:%02d\n",
               local_time->tm_hour, local_time->tm_min, local_time->tm_sec);
        printf("[VIRT_RTC] Current date: %04d-%02d-%02d\n",
               local_time->tm_year + 1900, local_time->tm_mon + 1, local_time->tm_mday);
    }
}

void virt_rtc_deinit(virt_rtc_t* rtc) {
    if (rtc && rtc->initialized) {
        rtc->initialized = false;
        printf("[VIRT_RTC] Deinitialized\n");
    }
}

void virt_rtc_set_time(virt_rtc_t* rtc, uint8_t hour, uint8_t minute, uint8_t second) {
    if (!rtc || !rtc->initialized) {
        printf("[VIRT_RTC] Error: RTC not initialized\n");
        return;
    }
    
    if (hour > 23 || minute > 59 || second > 59) {
        printf("[VIRT_RTC] Error: Invalid time values\n");
        return;
    }
    
    rtc->TR = ((hour / 10) << 20) |
              ((hour % 10) << 16) |
              ((minute / 10) << 12) |
              ((minute % 10) << 8) |
              ((second / 10) << 4) |
              (second % 10);
    
    printf("[VIRT_RTC] Time set to: %02u:%02u:%02u\n", hour, minute, second);
}

void virt_rtc_set_date(virt_rtc_t* rtc, uint8_t year, uint8_t month, 
                       uint8_t day, uint8_t weekday) {
    if (!rtc || !rtc->initialized) {
        printf("[VIRT_RTC] Error: RTC not initialized\n");
        return;
    }
    
    if (year > 99 || month > 12 || month < 1 || day > 31 || day < 1 || weekday > 6) {
        printf("[VIRT_RTC] Error: Invalid date values\n");
        return;
    }
    
    rtc->DR = ((year / 10) << 20) |
              ((year % 10) << 16) |
              ((month / 10) << 12) |
              ((month % 10) << 8) |
              ((day / 10) << 4) |
              (day % 10);
    
    printf("[VIRT_RTC] Date set to: 20%02u-%02u-%02u (Weekday: %u)\n",
           year, month, day, weekday);
}

void virt_rtc_get_time(virt_rtc_t* rtc, uint8_t* hour, uint8_t* minute, uint8_t* second) {
    if (!rtc || !rtc->initialized || !hour || !minute || !second) {
        printf("[VIRT_RTC] Error: Invalid parameters\n");
        return;
    }
    
    *hour = ((rtc->TR >> 20) & 0x03) * 10 + ((rtc->TR >> 16) & 0x0F);
    *minute = ((rtc->TR >> 12) & 0x07) * 10 + ((rtc->TR >> 8) & 0x0F);
    *second = ((rtc->TR >> 4) & 0x07) * 10 + (rtc->TR & 0x0F);
}

void virt_rtc_get_date(virt_rtc_t* rtc, uint8_t* year, uint8_t* month, 
                       uint8_t* day, uint8_t* weekday) {
    if (!rtc || !rtc->initialized || !year || !month || !day) {
        printf("[VIRT_RTC] Error: Invalid parameters\n");
        return;
    }
    
    *year = ((rtc->DR >> 20) & 0x0F) * 10 + ((rtc->DR >> 16) & 0x0F);
    *month = ((rtc->DR >> 12) & 0x01) * 10 + ((rtc->DR >> 8) & 0x0F);
    *day = ((rtc->DR >> 4) & 0x03) * 10 + (rtc->DR & 0x0F);
    
    if (weekday) {
        *weekday = (rtc->DR >> 13) & 0x07;
    }
}

uint32_t virt_rtc_get_timestamp(virt_rtc_t* rtc) {
    if (!rtc || !rtc->initialized) {
        return 0;
    }
    
    // Convert RTC time to Unix timestamp
    time_t now = time(NULL);
    return (uint32_t)now;
}