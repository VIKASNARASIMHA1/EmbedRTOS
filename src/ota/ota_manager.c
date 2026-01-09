#include "ota_manager.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// OTA Manager context
static ota_manager_t ota_ctx;

// Statistics
static uint32_t total_updates = 0;
static uint32_t successful_updates = 0;
static uint32_t failed_updates = 0;

// Callbacks
static ota_progress_callback_t progress_callback = NULL;
static ota_status_callback_t status_callback = NULL;
static ota_validate_callback_t validate_callback = NULL;

// Helper function to update progress
static void update_progress(uint8_t percent) {
    ota_ctx.progress_percent = percent;
    if (progress_callback) {
        progress_callback(percent);
    }
}

// Helper function to update state
static void update_state(ota_state_t new_state, ota_error_t error) {
    ota_ctx.state = new_state;
    ota_ctx.last_error = error;
    
    if (status_callback) {
        status_callback(new_state, error);
    }
    
    printf("[OTA_MANAGER] State changed to: %s", ota_manager_state_to_string(new_state));
    if (error != OTA_ERROR_NONE) {
        printf(" (Error: %s)", ota_manager_error_to_string(error));
    }
    printf("\n");
}

void ota_manager_init(void) {
    memset(&ota_ctx, 0, sizeof(ota_ctx));
    
    ota_ctx.state = OTA_STATE_IDLE;
    ota_ctx.last_error = OTA_ERROR_NONE;
    ota_ctx.update_address = APP_SLOT_B_ADDRESS;
    ota_ctx.timeout_ms = 30000; // 30 second timeout
    ota_ctx.progress_percent = 0;
    
    printf("[OTA_MANAGER] Initialized\n");
    printf("[OTA_MANAGER] Update address: 0x%08X\n", ota_ctx.update_address);
    printf("[OTA_MANAGER] Timeout: %u ms\n", ota_ctx.timeout_ms);
}

ota_error_t ota_manager_start_update(uint32_t total_size, uint32_t chunk_size) {
    if (ota_ctx.state != OTA_STATE_IDLE) {
        printf("[OTA_MANAGER] Error: Cannot start update, manager is not idle\n");
        return OTA_ERROR_INVALID_STATE;
    }
    
    if (total_size == 0 || chunk_size == 0 || chunk_size > 1024) {
        printf("[OTA_MANAGER] Error: Invalid size parameters\n");
        return OTA_ERROR_INVALID_SIZE;
    }
    
    if (total_size > (FLASH_TOTAL_SIZE - APP_SLOT_B_ADDRESS)) {
        printf("[OTA_MANAGER] Error: Firmware too large\n");
        return OTA_ERROR_INVALID_SIZE;
    }
    
    // Initialize chunk info
    ota_ctx.chunk_info.total_size = total_size;
    ota_ctx.chunk_info.chunk_size = chunk_size;
    ota_ctx.chunk_info.total_chunks = (total_size + chunk_size - 1) / chunk_size;
    ota_ctx.chunk_info.received_chunks = 0;
    ota_ctx.chunk_info.received_bytes = 0;
    ota_ctx.chunk_info.next_expected_chunk = 0;
    
    ota_ctx.start_time = (uint32_t)time(NULL);
    ota_ctx.last_activity_time = ota_ctx.start_time;
    ota_ctx.abort_requested = false;
    ota_ctx.restart_required = false;
    
    // Erase the update area
    printf("[OTA_MANAGER] Erasing update area...\n");
    bootloader_error_t boot_error = flash_erase_page(APP_SLOT_B_ADDRESS);
    if (boot_error != BOOT_OK) {
        printf("[OTA_MANAGER] Error: Failed to erase flash\n");
        update_state(OTA_STATE_ERROR, OTA_ERROR_FLASH_ERROR);
        return OTA_ERROR_FLASH_ERROR;
    }
    
    total_updates++;
    update_state(OTA_STATE_WAITING_FOR_START, OTA_ERROR_NONE);
    update_progress(0);
    
    printf("[OTA_MANAGER] Update started\n");
    printf("  Total size: %u bytes\n", total_size);
    printf("  Chunk size: %u bytes\n", chunk_size);
    printf("  Total chunks: %u\n", ota_ctx.chunk_info.total_chunks);
    printf("  Target address: 0x%08X\n", APP_SLOT_B_ADDRESS);
    
    return OTA_ERROR_NONE;
}

ota_error_t ota_manager_receive_chunk(uint32_t chunk_number, const uint8_t* data, uint32_t size) {
    if (ota_ctx.state != OTA_STATE_WAITING_FOR_START && 
        ota_ctx.state != OTA_STATE_RECEIVING_DATA) {
        printf("[OTA_MANAGER] Error: Not ready to receive data\n");
        return OTA_ERROR_INVALID_STATE;
    }
    
    if (chunk_number != ota_ctx.chunk_info.next_expected_chunk) {
        printf("[OTA_MANAGER] Error: Unexpected chunk number. Expected %u, got %u\n",
               ota_ctx.chunk_info.next_expected_chunk, chunk_number);
        return OTA_ERROR_COMMUNICATION;
    }
    
    if (data == NULL || size == 0 || size > ota_ctx.chunk_info.chunk_size) {
        printf("[OTA_MANAGER] Error: Invalid chunk data\n");
        return OTA_ERROR_INVALID_SIZE;
    }
    
    if (ota_ctx.chunk_info.received_bytes + size > ota_ctx.chunk_info.total_size) {
        printf("[OTA_MANAGER] Error: Chunk exceeds total size\n");
        return OTA_ERROR_INVALID_SIZE;
    }
    
    // Update state if this is the first chunk
    if (ota_ctx.state == OTA_STATE_WAITING_FOR_START) {
        update_state(OTA_STATE_RECEIVING_DATA, OTA_ERROR_NONE);
    }
    
    // Calculate write address
    uint32_t write_address = APP_SLOT_B_ADDRESS + ota_ctx.chunk_info.received_bytes;
    
    printf("[OTA_MANAGER] Receiving chunk %u/%u (%u bytes) -> 0x%08X\n",
           chunk_number + 1, ota_ctx.chunk_info.total_chunks, size, write_address);
    
    // Write data to flash
    for (uint32_t i = 0; i < size; i += 4) {
        uint32_t word = 0;
        uint32_t bytes_to_write = (size - i) >= 4 ? 4 : (size - i);
        
        // Construct word from bytes
        for (uint32_t j = 0; j < bytes_to_write; j++) {
            word |= (data[i + j] << (j * 8));
        }
        
        bootloader_error_t error = flash_write_word(write_address + i, word);
        if (error != BOOT_OK) {
            printf("[OTA_MANAGER] Error: Failed to write to flash at 0x%08X\n",
                   write_address + i);
            update_state(OTA_STATE_ERROR, OTA_ERROR_FLASH_ERROR);
            return OTA_ERROR_FLASH_ERROR;
        }
    }
    
    // Update statistics
    ota_ctx.chunk_info.received_chunks++;
    ota_ctx.chunk_info.received_bytes += size;
    ota_ctx.chunk_info.next_expected_chunk++;
    ota_ctx.last_activity_time = (uint32_t)time(NULL);
    
    // Update progress
    uint8_t progress = (uint8_t)((ota_ctx.chunk_info.received_bytes * 100) / 
                                 ota_ctx.chunk_info.total_size);
    update_progress(progress);
    
    printf("[OTA_MANAGER] Chunk %u received successfully\n", chunk_number);
    printf("  Received: %u/%u bytes (%.1f%%)\n",
           ota_ctx.chunk_info.received_bytes, ota_ctx.chunk_info.total_size,
           (float)ota_ctx.chunk_info.received_bytes * 100.0f / ota_ctx.chunk_info.total_size);
    
    return OTA_ERROR_NONE;
}

ota_error_t ota_manager_finalize_update(void) {
    if (ota_ctx.state != OTA_STATE_RECEIVING_DATA) {
        printf("[OTA_MANAGER] Error: Not in receiving state\n");
        return OTA_ERROR_INVALID_STATE;
    }
    
    if (ota_ctx.chunk_info.received_bytes != ota_ctx.chunk_info.total_size) {
        printf("[OTA_MANAGER] Error: Incomplete transfer. Received %u/%u bytes\n",
               ota_ctx.chunk_info.received_bytes, ota_ctx.chunk_info.total_size);
        return OTA_ERROR_INVALID_SIZE;
    }
    
    update_state(OTA_STATE_VALIDATING, OTA_ERROR_NONE);
    update_progress(95);
    
    printf("[OTA_MANAGER] All chunks received (%u bytes total)\n", 
           ota_ctx.chunk_info.received_bytes);
    printf("[OTA_MANAGER] Starting validation...\n");
    
    // Validate the received firmware
    return ota_manager_validate_update();
}

ota_error_t ota_manager_validate_update(void) {
    printf("[OTA_MANAGER] Validating firmware...\n");
    
    // Read firmware header
    memcpy(&ota_ctx.firmware_header, &flash_memory[APP_SLOT_B_ADDRESS], 
           sizeof(firmware_header_t));
    
    // Validate header
    if (!validate_firmware_header(&ota_ctx.firmware_header)) {
        printf("[OTA_MANAGER] Error: Invalid firmware header\n");
        update_state(OTA_STATE_ERROR, OTA_ERROR_VALIDATION_FAILED);
        failed_updates++;
        return OTA_ERROR_VALIDATION_FAILED;
    }
    
    // Check size matches
    if (ota_ctx.firmware_header.size != ota_ctx.chunk_info.total_size) {
        printf("[OTA_MANAGER] Error: Size mismatch. Header: %u, Received: %u\n",
               ota_ctx.firmware_header.size, ota_ctx.chunk_info.total_size);
        update_state(OTA_STATE_ERROR, OTA_ERROR_VALIDATION_FAILED);
        failed_updates++;
        return OTA_ERROR_VALIDATION_FAILED;
    }
    
    // Calculate CRC32
    uint32_t calculated_crc = calculate_crc32(
        &flash_memory[APP_SLOT_B_ADDRESS + sizeof(firmware_header_t)],
        ota_ctx.firmware_header.size - sizeof(firmware_header_t));
    
    if (calculated_crc != ota_ctx.firmware_header.crc32) {
        printf("[OTA_MANAGER] Error: CRC mismatch\n");
        printf("  Calculated: 0x%08X\n", calculated_crc);
        printf("  Expected:   0x%08X\n", ota_ctx.firmware_header.crc32);
        update_state(OTA_STATE_ERROR, OTA_ERROR_CRC_MISMATCH);
        failed_updates++;
        return OTA_ERROR_CRC_MISMATCH;
    }
    
    // Call application validation callback if set
    if (validate_callback && !validate_callback(&ota_ctx.firmware_header)) {
        printf("[OTA_MANAGER] Error: Application validation failed\n");
        update_state(OTA_STATE_ERROR, OTA_ERROR_VALIDATION_FAILED);
        failed_updates++;
        return OTA_ERROR_VALIDATION_FAILED;
    }
    
    printf("[OTA_MANAGER] Firmware validation successful!\n");
    printf("  Version: %s\n", ota_ctx.firmware_header.version_string);
    printf("  Size: %u bytes\n", ota_ctx.firmware_header.size);
    printf("  CRC32: 0x%08X ✓\n", ota_ctx.firmware_header.crc32);
    
    update_state(OTA_STATE_COMPLETE, OTA_ERROR_NONE);
    update_progress(100);
    successful_updates++;
    
    ota_ctx.restart_required = true;
    
    return OTA_ERROR_NONE;
}

ota_error_t ota_manager_abort_update(void) {
    printf("[OTA_MANAGER] Aborting update...\n");
    
    ota_ctx.abort_requested = true;
    failed_updates++;
    
    update_state(OTA_STATE_ERROR, OTA_ERROR_COMMUNICATION);
    
    printf("[OTA_MANAGER] Update aborted\n");
    
    return OTA_ERROR_NONE;
}

void ota_manager_apply_update(void) {
    if (ota_ctx.state != OTA_STATE_COMPLETE) {
        printf("[OTA_MANAGER] Error: Cannot apply incomplete update\n");
        return;
    }
    
    printf("[OTA_MANAGER] Applying update...\n");
    update_state(OTA_STATE_UPDATING, OTA_ERROR_NONE);
    
    // Switch to the new firmware
    bootloader_error_t error = bootloader_switch_to_update();
    if (error != BOOT_OK) {
        printf("[OTA_MANAGER] Error: Failed to switch to update: %d\n", error);
        update_state(OTA_STATE_ERROR, OTA_ERROR_FLASH_ERROR);
        return;
    }
    
    printf("[OTA_MANAGER] Update applied successfully!\n");
    printf("[OTA_MANAGER] System restart required to run new firmware\n");
    
    ota_ctx.restart_required = true;
}

// Status and control functions
ota_state_t ota_manager_get_state(void) {
    return ota_ctx.state;
}

ota_error_t ota_manager_get_last_error(void) {
    return ota_ctx.last_error;
}

uint8_t ota_manager_get_progress(void) {
    return ota_ctx.progress_percent;
}

bool ota_manager_is_busy(void) {
    return ota_ctx.state != OTA_STATE_IDLE && 
           ota_ctx.state != OTA_STATE_COMPLETE &&
           ota_ctx.state != OTA_STATE_ERROR;
}

bool ota_manager_is_update_available(void) {
    return ota_ctx.restart_required;
}

const char* ota_manager_state_to_string(ota_state_t state) {
    switch (state) {
        case OTA_STATE_IDLE: return "IDLE";
        case OTA_STATE_WAITING_FOR_START: return "WAITING_FOR_START";
        case OTA_STATE_RECEIVING_DATA: return "RECEIVING_DATA";
        case OTA_STATE_VALIDATING: return "VALIDATING";
        case OTA_STATE_UPDATING: return "UPDATING";
        case OTA_STATE_COMPLETE: return "COMPLETE";
        case OTA_STATE_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

const char* ota_manager_error_to_string(ota_error_t error) {
    switch (error) {
        case OTA_ERROR_NONE: return "NONE";
        case OTA_ERROR_INVALID_STATE: return "INVALID_STATE";
        case OTA_ERROR_INVALID_SIZE: return "INVALID_SIZE";
        case OTA_ERROR_CRC_MISMATCH: return "CRC_MISMATCH";
        case OTA_ERROR_FLASH_ERROR: return "FLASH_ERROR";
        case OTA_ERROR_TIMEOUT: return "TIMEOUT";
        case OTA_ERROR_COMMUNICATION: return "COMMUNICATION";
        case OTA_ERROR_VALIDATION_FAILED: return "VALIDATION_FAILED";
        default: return "UNKNOWN_ERROR";
    }
}

// Statistics functions
void ota_manager_print_status(void) {
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║                 OTA MANAGER STATUS                   ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║ State:            %-32s ║\n", ota_manager_state_to_string(ota_ctx.state));
    printf("║ Last Error:       %-32s ║\n", ota_manager_error_to_string(ota_ctx.last_error));
    printf("║ Progress:         %-32u%% ║\n", ota_ctx.progress_percent);
    printf("║ Received Bytes:   %-32u ║\n", ota_ctx.chunk_info.received_bytes);
    printf("║ Total Bytes:      %-32u ║\n", ota_ctx.chunk_info.total_size);
    printf("║ Received Chunks:  %-32u ║\n", ota_ctx.chunk_info.received_chunks);
    printf("║ Total Chunks:     %-32u ║\n", ota_ctx.chunk_info.total_chunks);
    printf("║ Restart Required: %-32s ║\n", ota_ctx.restart_required ? "YES" : "NO");
    printf("║ Abort Requested:  %-32s ║\n", ota_ctx.abort_requested ? "YES" : "NO");
    printf("╚══════════════════════════════════════════════════════╝\n");
}

void ota_manager_print_statistics(void) {
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║                OTA MANAGER STATISTICS                ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║ Total Updates:    %-32u ║\n", total_updates);
    printf("║ Successful:       %-32u ║\n", successful_updates);
    printf("║ Failed:           %-32u ║\n", failed_updates);
    printf("║ Success Rate:     %-31.1f%% ║\n", 
           total_updates > 0 ? (float)successful_updates * 100.0f / total_updates : 0.0f);
    printf("║ Current Progress: %-32u%% ║\n", ota_ctx.progress_percent);
    printf("╚══════════════════════════════════════════════════════╝\n");
}

uint32_t ota_manager_get_update_count(void) {
    return total_updates;
}

uint32_t ota_manager_get_successful_updates(void) {
    return successful_updates;
}

uint32_t ota_manager_get_failed_updates(void) {
    return failed_updates;
}

// Callback setters
void ota_manager_set_progress_callback(ota_progress_callback_t callback) {
    progress_callback = callback;
}

void ota_manager_set_status_callback(ota_status_callback_t callback) {
    status_callback = callback;
}

void ota_manager_set_validate_callback(ota_validate_callback_t callback) {
    validate_callback = callback;
}