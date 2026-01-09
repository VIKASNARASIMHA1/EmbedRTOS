#include "bootloader.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Bootloader context
static bootloader_context_t bootloader_ctx;
static uint8_t flash_memory[FLASH_TOTAL_SIZE];

// Simulated firmware images
static const uint8_t simulated_firmware_a[] = {
    0xDE, 0xAD, 0xBE, 0xEF, // magic
    0x01, 0x00, 0x00, 0x00, // version_major = 1
    0x00, 0x00, 0x00, 0x00, // version_minor = 0
    0x00, 0x00, 0x00, 0x00, // version_patch = 0
    'v', '1', '.', '0', '.', '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // version_string
    0x00, 0x00, 0x00, 0x00, // timestamp
    0x00, 0x10, 0x00, 0x00, // size = 4096
    0xAA, 0xBB, 0xCC, 0xDD, // crc32
    0x00, 0x40, 0x00, 0x08, // entry_point = 0x08004000
    // reserved[32]
};

static const uint8_t simulated_firmware_b[] = {
    0xDE, 0xAD, 0xBE, 0xEF, // magic
    0x01, 0x00, 0x00, 0x00, // version_major = 1
    0x01, 0x00, 0x00, 0x00, // version_minor = 1
    0x00, 0x00, 0x00, 0x00, // version_patch = 0
    'v', '1', '.', '1', '.', '0', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, // version_string
    0x00, 0x00, 0x00, 0x00, // timestamp
    0x00, 0x10, 0x00, 0x00, // size = 4096
    0xEE, 0xFF, 0x00, 0x11, // crc32
    0x00, 0x20, 0x00, 0x08, // entry_point = 0x08020000
    // reserved[32]
};

void bootloader_init(void) {
    memset(&bootloader_ctx, 0, sizeof(bootloader_ctx));
    memset(flash_memory, 0xFF, sizeof(flash_memory)); // Flash is 0xFF when erased
    
    bootloader_ctx.state = BOOT_STATE_INIT;
    bootloader_ctx.last_error = BOOT_OK;
    bootloader_ctx.boot_count = 1;
    bootloader_ctx.active_slot = APP_SLOT_A_ADDRESS;
    bootloader_ctx.update_slot = APP_SLOT_B_ADDRESS;
    bootloader_ctx.last_boot_time = (uint32_t)time(NULL);
    
    // Initialize simulated flash with firmware images
    memcpy(&flash_memory[APP_SLOT_A_ADDRESS], simulated_firmware_a, sizeof(simulated_firmware_a));
    memcpy(&flash_memory[APP_SLOT_B_ADDRESS], simulated_firmware_b, sizeof(simulated_firmware_b));
    
    // Load active header
    memcpy(&bootloader_ctx.active_header, &flash_memory[APP_SLOT_A_ADDRESS], sizeof(firmware_header_t));
    
    printf("[BOOTLOADER] Initialized\n");
    printf("[BOOTLOADER] Active slot: 0x%08X\n", bootloader_ctx.active_slot);
    printf("[BOOTLOADER] Update slot: 0x%08X\n", bootloader_ctx.update_slot);
    printf("[BOOTLOADER] Boot count: %u\n", bootloader_ctx.boot_count);
}

bootloader_error_t bootloader_check_update(void) {
    printf("[BOOTLOADER] Checking for updates...\n");
    
    bootloader_ctx.state = BOOT_STATE_CHECK_UPDATE;
    
    // Check if update slot contains valid firmware
    bootloader_error_t error = bootloader_validate_firmware(APP_SLOT_B_ADDRESS);
    
    if (error == BOOT_OK) {
        // Load update header
        memcpy(&bootloader_ctx.update_header, &flash_memory[APP_SLOT_B_ADDRESS], sizeof(firmware_header_t));
        
        // Check if update is newer than current
        uint32_t current_version = (bootloader_ctx.active_header.version_major << 16) |
                                   (bootloader_ctx.active_header.version_minor << 8) |
                                   bootloader_ctx.active_header.version_patch;
        
        uint32_t update_version = (bootloader_ctx.update_header.version_major << 16) |
                                  (bootloader_ctx.update_header.version_minor << 8) |
                                  bootloader_ctx.update_header.version_patch;
        
        if (update_version > current_version) {
            printf("[BOOTLOADER] Update available!\n");
            printf("  Current: v%u.%u.%u\n", 
                   bootloader_ctx.active_header.version_major,
                   bootloader_ctx.active_header.version_minor,
                   bootloader_ctx.active_header.version_patch);
            printf("  Update:  v%u.%u.%u\n",
                   bootloader_ctx.update_header.version_major,
                   bootloader_ctx.update_header.version_minor,
                   bootloader_ctx.update_header.version_patch);
            
            bootloader_ctx.update_pending = true;
            return BOOT_OK;
        } else {
            printf("[BOOTLOADER] No newer update available\n");
            bootloader_ctx.update_pending = false;
            return BOOT_OK;
        }
    } else {
        printf("[BOOTLOADER] No valid update found\n");
        bootloader_ctx.update_pending = false;
        return error;
    }
}

bootloader_error_t bootloader_validate_firmware(uint32_t address) {
    if (address >= FLASH_TOTAL_SIZE) {
        return BOOT_ERROR_INVALID_SIZE;
    }
    
    firmware_header_t header;
    memcpy(&header, &flash_memory[address], sizeof(firmware_header_t));
    
    // Check magic number
    if (header.magic != FIRMWARE_MAGIC) {
        printf("[BOOTLOADER] Invalid magic: 0x%08X (expected: 0x%08X)\n",
               header.magic, FIRMWARE_MAGIC);
        return BOOT_ERROR_MAGIC_MISMATCH;
    }
    
    // Check size
    if (header.size == 0 || header.size > FLASH_TOTAL_SIZE - address) {
        printf("[BOOTLOADER] Invalid size: %u\n", header.size);
        return BOOT_ERROR_INVALID_SIZE;
    }
    
    // Calculate CRC32 of firmware (excluding header)
    uint32_t calculated_crc = calculate_crc32(&flash_memory[address + sizeof(firmware_header_t)], 
                                             header.size - sizeof(firmware_header_t));
    
    if (calculated_crc != header.crc32) {
        printf("[BOOTLOADER] CRC mismatch: calculated=0x%08X, stored=0x%08X\n",
               calculated_crc, header.crc32);
        return BOOT_ERROR_CRC_MISMATCH;
    }
    
    printf("[BOOTLOADER] Firmware at 0x%08X is valid\n", address);
    printf("  Version: v%u.%u.%u (%s)\n",
           header.version_major, header.version_minor, header.version_patch,
           header.version_string);
    printf("  Size: %u bytes\n", header.size);
    printf("  CRC32: 0x%08X ✓\n", header.crc32);
    printf("  Entry point: 0x%08X\n", header.entry_point);
    
    return BOOT_OK;
}

bootloader_error_t bootloader_switch_to_update(void) {
    if (!bootloader_ctx.update_pending) {
        printf("[BOOTLOADER] No update pending\n");
        return BOOT_ERROR_NO_APP;
    }
    
    printf("[BOOTLOADER] Switching to update...\n");
    bootloader_ctx.state = BOOT_STATE_UPDATE_IN_PROGRESS;
    
    // Validate update firmware
    bootloader_error_t error = bootloader_validate_firmware(APP_SLOT_B_ADDRESS);
    if (error != BOOT_OK) {
        bootloader_ctx.last_error = error;
        bootloader_ctx.state = BOOT_STATE_ERROR;
        printf("[BOOTLOADER] Update validation failed: %d\n", error);
        return error;
    }
    
    // For simulation, just swap the active slot
    bootloader_ctx.active_slot = APP_SLOT_B_ADDRESS;
    memcpy(&bootloader_ctx.active_header, &bootloader_ctx.update_header, sizeof(firmware_header_t));
    
    bootloader_ctx.update_pending = false;
    bootloader_ctx.boot_count++;
    
    printf("[BOOTLOADER] Successfully switched to update!\n");
    printf("[BOOTLOADER] New active slot: 0x%08X\n", bootloader_ctx.active_slot);
    printf("[BOOTLOADER] New version: %s\n", bootloader_ctx.active_header.version_string);
    
    return BOOT_OK;
}

bootloader_error_t bootloader_rollback(void) {
    printf("[BOOTLOADER] Rolling back to previous version...\n");
    
    // Validate slot A firmware
    bootloader_error_t error = bootloader_validate_firmware(APP_SLOT_A_ADDRESS);
    if (error != BOOT_OK) {
        printf("[BOOTLOADER] Rollback validation failed: %d\n", error);
        return error;
    }
    
    // Switch back to slot A
    bootloader_ctx.active_slot = APP_SLOT_A_ADDRESS;
    memcpy(&bootloader_ctx.active_header, &flash_memory[APP_SLOT_A_ADDRESS], sizeof(firmware_header_t));
    
    bootloader_ctx.rollback_requested = true;
    bootloader_ctx.boot_count++;
    
    printf("[BOOTLOADER] Rollback successful!\n");
    printf("[BOOTLOADER] Active slot: 0x%08X\n", bootloader_ctx.active_slot);
    printf("[BOOTLOADER] Version: %s\n", bootloader_ctx.active_header.version_string);
    
    return BOOT_OK;
}

void bootloader_jump_to_app(uint32_t address) {
    printf("[BOOTLOADER] Jumping to application at 0x%08X\n", address);
    
    if (address == APP_SLOT_A_ADDRESS) {
        printf("[BOOTLOADER] Starting Application Slot A (v1.0.0)\n");
    } else if (address == APP_SLOT_B_ADDRESS) {
        printf("[BOOTLOADER] Starting Application Slot B (v1.1.0)\n");
    } else {
        printf("[BOOTLOADER] Error: Invalid application address\n");
        return;
    }
    
    // In a real system, this would set the stack pointer and jump
    // For simulation, we just print the action
    printf("[BOOTLOADER] Setting SP = 0x%08X\n", flash_read_word(address));
    printf("[BOOTLOADER] Setting PC = 0x%08X\n", flash_read_word(address + 4));
    printf("[BOOTLOADER] Application started!\n");
    
    bootloader_ctx.state = BOOT_STATE_JUMP_TO_APP;
}

uint32_t bootloader_get_active_slot(void) {
    return bootloader_ctx.active_slot;
}

const char* bootloader_get_version_string(void) {
    static char version[32];
    snprintf(version, sizeof(version), "v%u.%u.%u",
             bootloader_ctx.active_header.version_major,
             bootloader_ctx.active_header.version_minor,
             bootloader_ctx.active_header.version_patch);
    return version;
}

void bootloader_print_status(void) {
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║                 BOOTLOADER STATUS                    ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║ State:            %-32s ║\n", 
           bootloader_ctx.state == BOOT_STATE_INIT ? "INIT" :
           bootloader_ctx.state == BOOT_STATE_CHECK_UPDATE ? "CHECK_UPDATE" :
           bootloader_ctx.state == BOOT_STATE_VALIDATE_APP ? "VALIDATE_APP" :
           bootloader_ctx.state == BOOT_STATE_JUMP_TO_APP ? "JUMP_TO_APP" :
           bootloader_ctx.state == BOOT_STATE_UPDATE_IN_PROGRESS ? "UPDATE_IN_PROGRESS" :
           "ERROR");
    printf("║ Last Error:       %-32d ║\n", bootloader_ctx.last_error);
    printf("║ Active Slot:      0x%-30X ║\n", bootloader_ctx.active_slot);
    printf("║ Update Slot:      0x%-30X ║\n", bootloader_ctx.update_slot);
    printf("║ Boot Count:       %-32u ║\n", bootloader_ctx.boot_count);
    printf("║ Update Pending:   %-32s ║\n", bootloader_ctx.update_pending ? "YES" : "NO");
    printf("║ Rollback Request: %-32s ║\n", bootloader_ctx.rollback_requested ? "YES" : "NO");
    printf("║ Version:          %-32s ║\n", bootloader_get_version_string());
    printf("╚══════════════════════════════════════════════════════╝\n");
    
    printf("\nFirmware Details:\n");
    bootloader_print_header(&bootloader_ctx.active_header);
}

void bootloader_print_header(const firmware_header_t* header) {
    if (!header) return;
    
    printf("  Magic:        0x%08X\n", header->magic);
    printf("  Version:      v%u.%u.%u\n", header->version_major, 
           header->version_minor, header->version_patch);
    printf("  Version Str:  %s\n", header->version_string);
    printf("  Timestamp:    %u\n", header->timestamp);
    printf("  Size:         %u bytes\n", header->size);
    printf("  CRC32:        0x%08X\n", header->crc32);
    printf("  Entry Point:  0x%08X\n", header->entry_point);
}

// Flash simulation functions
bootloader_error_t flash_erase_page(uint32_t address) {
    if (address >= FLASH_TOTAL_SIZE) {
        return BOOT_ERROR_FLASH_ERASE;
    }
    
    uint32_t page_start = address & ~(FLASH_PAGE_SIZE - 1);
    
    printf("[FLASH] Erasing page at 0x%08X...\n", page_start);
    
    memset(&flash_memory[page_start], 0xFF, FLASH_PAGE_SIZE);
    
    printf("[FLASH] Page erased successfully\n");
    return BOOT_OK;
}

bootloader_error_t flash_write_word(uint32_t address, uint32_t data) {
    if (address >= FLASH_TOTAL_SIZE - 3) {
        return BOOT_ERROR_FLASH_WRITE;
    }
    
    printf("[FLASH] Writing 0x%08X to 0x%08X\n", data, address);
    
    // Check if location is erased (all 0xFF)
    for (int i = 0; i < 4; i++) {
        if (flash_memory[address + i] != 0xFF) {
            printf("[FLASH] Error: Location not erased (0x%02X at 0x%08X)\n",
                   flash_memory[address + i], address + i);
            return BOOT_ERROR_FLASH_WRITE;
        }
    }
    
    // Write little-endian
    flash_memory[address] = data & 0xFF;
    flash_memory[address + 1] = (data >> 8) & 0xFF;
    flash_memory[address + 2] = (data >> 16) & 0xFF;
    flash_memory[address + 3] = (data >> 24) & 0xFF;
    
    printf("[FLASH] Write successful\n");
    return BOOT_OK;
}

uint32_t flash_read_word(uint32_t address) {
    if (address >= FLASH_TOTAL_SIZE - 3) {
        return 0xFFFFFFFF;
    }
    
    uint32_t value = flash_memory[address] |
                    (flash_memory[address + 1] << 8) |
                    (flash_memory[address + 2] << 16) |
                    (flash_memory[address + 3] << 24);
    
    return value;
}

void flash_dump(uint32_t address, uint32_t size) {
    if (address >= FLASH_TOTAL_SIZE || size == 0 || 
        address + size > FLASH_TOTAL_SIZE) {
        printf("[FLASH] Invalid dump parameters\n");
        return;
    }
    
    printf("[FLASH] Dump from 0x%08X to 0x%08X (%u bytes):\n",
           address, address + size - 1, size);
    
    for (uint32_t i = 0; i < size; i += 16) {
        printf("  0x%08X: ", address + i);
        
        // Print hex
        for (uint32_t j = 0; j < 16 && i + j < size; j++) {
            printf("%02X ", flash_memory[address + i + j]);
            if (j == 7) printf(" ");
        }
        
        // Print ASCII
        printf(" ");
        for (uint32_t j = 0; j < 16 && i + j < size; j++) {
            uint8_t c = flash_memory[address + i + j];
            if (c >= 32 && c <= 126) {
                printf("%c", c);
            } else {
                printf(".");
            }
        }
        printf("\n");
    }
}

// CRC32 calculation (simplified)
uint32_t calculate_crc32(const uint8_t* data, uint32_t length) {
    uint32_t crc = 0xFFFFFFFF;
    
    for (uint32_t i = 0; i < length; i++) {
        crc ^= data[i];
        for (int j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xEDB88320;
            } else {
                crc >>= 1;
            }
        }
    }
    
    return ~crc;
}

bool validate_firmware_header(const firmware_header_t* header) {
    return (header->magic == FIRMWARE_MAGIC) &&
           (header->size > 0) &&
           (header->entry_point != 0);
}

uint32_t get_current_timestamp(void) {
    return (uint32_t)time(NULL);
}