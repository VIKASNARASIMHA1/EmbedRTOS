#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include <stdint.h>
#include <stdbool.h>

// Flash memory layout (simulated)
#define BOOTLOADER_SIZE         0x4000      // 16KB
#define APP_SLOT_A_ADDRESS      0x08004000  // Application slot A
#define APP_SLOT_B_ADDRESS      0x08020000  // Application slot B (OTA slot)
#define FLASH_PAGE_SIZE         0x800       // 2KB
#define FLASH_TOTAL_SIZE        0x100000    // 1MB

// Firmware metadata
#define FIRMWARE_MAGIC          0xDEADBEEF
#define FIRMWARE_VERSION_LEN    16

typedef struct {
    uint32_t magic;
    uint32_t version_major;
    uint32_t version_minor;
    uint32_t version_patch;
    char version_string[FIRMWARE_VERSION_LEN];
    uint32_t timestamp;
    uint32_t size;
    uint32_t crc32;
    uint32_t entry_point;
    uint8_t reserved[32];
} firmware_header_t;

// Bootloader states
typedef enum {
    BOOT_STATE_INIT,
    BOOT_STATE_CHECK_UPDATE,
    BOOT_STATE_VALIDATE_APP,
    BOOT_STATE_JUMP_TO_APP,
    BOOT_STATE_UPDATE_IN_PROGRESS,
    BOOT_STATE_ERROR
} bootloader_state_t;

// Bootloader error codes
typedef enum {
    BOOT_OK = 0,
    BOOT_ERROR_NO_APP,
    BOOT_ERROR_CRC_MISMATCH,
    BOOT_ERROR_MAGIC_MISMATCH,
    BOOT_ERROR_FLASH_WRITE,
    BOOT_ERROR_FLASH_ERASE,
    BOOT_ERROR_INVALID_SIZE,
    BOOT_ERROR_UPDATE_IN_PROGRESS,
    BOOT_ERROR_ROLLBACK_FAILED
} bootloader_error_t;

// Bootloader context
typedef struct {
    bootloader_state_t state;
    bootloader_error_t last_error;
    uint32_t active_slot;
    uint32_t update_slot;
    firmware_header_t active_header;
    firmware_header_t update_header;
    uint32_t boot_count;
    bool update_pending;
    bool rollback_requested;
    uint32_t last_boot_time;
} bootloader_context_t;

// Function prototypes
void bootloader_init(void);
bootloader_error_t bootloader_check_update(void);
bootloader_error_t bootloader_validate_firmware(uint32_t address);
bootloader_error_t bootloader_switch_to_update(void);
bootloader_error_t bootloader_rollback(void);
void bootloader_jump_to_app(uint32_t address);
uint32_t bootloader_get_active_slot(void);
const char* bootloader_get_version_string(void);
void bootloader_print_status(void);
void bootloader_print_header(const firmware_header_t* header);

// Flash simulation functions (emulated)
bootloader_error_t flash_erase_page(uint32_t address);
bootloader_error_t flash_write_word(uint32_t address, uint32_t data);
uint32_t flash_read_word(uint32_t address);
void flash_dump(uint32_t address, uint32_t size);

// Utility functions
uint32_t calculate_crc32(const uint8_t* data, uint32_t length);
bool validate_firmware_header(const firmware_header_t* header);
uint32_t get_current_timestamp(void);

#endif