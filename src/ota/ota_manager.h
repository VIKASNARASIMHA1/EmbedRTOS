#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "bootloader.h"

// OTA Manager states
typedef enum {
    OTA_STATE_IDLE,
    OTA_STATE_WAITING_FOR_START,
    OTA_STATE_RECEIVING_DATA,
    OTA_STATE_VALIDATING,
    OTA_STATE_UPDATING,
    OTA_STATE_COMPLETE,
    OTA_STATE_ERROR
} ota_state_t;

// OTA Manager error codes
typedef enum {
    OTA_ERROR_NONE = 0,
    OTA_ERROR_INVALID_STATE,
    OTA_ERROR_INVALID_SIZE,
    OTA_ERROR_CRC_MISMATCH,
    OTA_ERROR_FLASH_ERROR,
    OTA_ERROR_TIMEOUT,
    OTA_ERROR_COMMUNICATION,
    OTA_ERROR_VALIDATION_FAILED
} ota_error_t;

// OTA chunk information
typedef struct {
    uint32_t total_size;
    uint32_t chunk_size;
    uint32_t total_chunks;
    uint32_t received_chunks;
    uint32_t received_bytes;
    uint32_t next_expected_chunk;
} ota_chunk_info_t;

// OTA Manager context
typedef struct {
    ota_state_t state;
    ota_error_t last_error;
    ota_chunk_info_t chunk_info;
    firmware_header_t firmware_header;
    uint32_t update_address;
    uint32_t start_time;
    uint32_t last_activity_time;
    uint32_t timeout_ms;
    bool abort_requested;
    bool restart_required;
    uint8_t progress_percent;
} ota_manager_t;

// Function prototypes
void ota_manager_init(void);
ota_error_t ota_manager_start_update(uint32_t total_size, uint32_t chunk_size);
ota_error_t ota_manager_receive_chunk(uint32_t chunk_number, 
                                      const uint8_t* data, uint32_t size);
ota_error_t ota_manager_finalize_update(void);
ota_error_t ota_manager_abort_update(void);
ota_error_t ota_manager_validate_update(void);
void ota_manager_apply_update(void);

// Status and control
ota_state_t ota_manager_get_state(void);
ota_error_t ota_manager_get_last_error(void);
uint8_t ota_manager_get_progress(void);
bool ota_manager_is_busy(void);
bool ota_manager_is_update_available(void);
const char* ota_manager_state_to_string(ota_state_t state);
const char* ota_manager_error_to_string(ota_error_t error);

// Statistics
void ota_manager_print_status(void);
void ota_manager_print_statistics(void);
uint32_t ota_manager_get_update_count(void);
uint32_t ota_manager_get_successful_updates(void);
uint32_t ota_manager_get_failed_updates(void);

// Callback functions (to be implemented by application)
typedef void (*ota_progress_callback_t)(uint8_t percent);
typedef void (*ota_status_callback_t)(ota_state_t state, ota_error_t error);
typedef bool (*ota_validate_callback_t)(const firmware_header_t* header);

void ota_manager_set_progress_callback(ota_progress_callback_t callback);
void ota_manager_set_status_callback(ota_status_callback_t callback);
void ota_manager_set_validate_callback(ota_validate_callback_t callback);

#endif