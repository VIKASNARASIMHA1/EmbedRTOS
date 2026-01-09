#ifndef COMM_PROTOCOL_H
#define COMM_PROTOCOL_H

#include <stdint.h>
#include <stdbool.h>

// Protocol constants
#define PROTOCOL_START_BYTE     0xAA
#define PROTOCOL_END_BYTE       0x55
#define PROTOCOL_ESCAPE_BYTE    0x7D
#define PROTOCOL_MAX_PACKET_SIZE 256
#define PROTOCOL_HEADER_SIZE    6
#define PROTOCOL_CRC_SIZE       2

// Command definitions
typedef enum {
    CMD_PING = 0x01,
    CMD_PONG = 0x02,
    CMD_GET_SENSOR_DATA = 0x10,
    CMD_SENSOR_DATA = 0x11,
    CMD_SET_CONFIG = 0x20,
    CMD_CONFIG_RESPONSE = 0x21,
    CMD_START_OTA = 0x30,
    CMD_OTA_DATA = 0x31,
    CMD_OTA_COMPLETE = 0x32,
    CMD_GET_STATUS = 0x40,
    CMD_STATUS_RESPONSE = 0x41,
    CMD_ERROR = 0xFF
} protocol_command_t;

// Error codes
typedef enum {
    ERROR_NONE = 0x00,
    ERROR_INVALID_CMD = 0x01,
    ERROR_INVALID_LEN = 0x02,
    ERROR_CRC_MISMATCH = 0x03,
    ERROR_BUSY = 0x04,
    ERROR_OTA_FAILED = 0x05,
    ERROR_SENSOR_FAILED = 0x06
} protocol_error_t;

// Packet structure
#pragma pack(push, 1)
typedef struct {
    uint8_t start_byte;
    uint8_t command;
    uint16_t sequence;
    uint16_t length;
    uint8_t data[PROTOCOL_MAX_PACKET_SIZE];
    uint16_t crc;
    uint8_t end_byte;
} protocol_packet_t;
#pragma pack(pop)

// Protocol handler structure
typedef struct {
    uint16_t sequence_counter;
    uint32_t bytes_sent;
    uint32_t bytes_received;
    uint32_t packets_sent;
    uint32_t packets_received;
    uint32_t crc_errors;
    uint32_t timeout_errors;
    bool initialized;
} protocol_handler_t;

// Function prototypes
void protocol_init(protocol_handler_t* handler);
void protocol_deinit(protocol_handler_t* handler);

// Packet creation
uint16_t protocol_create_packet(uint8_t command, const uint8_t* data, 
                                uint16_t data_len, uint8_t* buffer, 
                                uint16_t buffer_size, protocol_handler_t* handler);

// Packet parsing
bool protocol_parse_packet(const uint8_t* buffer, uint16_t buffer_len,
                          protocol_packet_t* packet, protocol_handler_t* handler);

// CRC calculation
uint16_t protocol_calculate_crc(const uint8_t* data, uint16_t length);

// Helper functions
const char* protocol_command_to_string(uint8_t command);
const char* protocol_error_to_string(uint8_t error);
void protocol_print_packet(const protocol_packet_t* packet, const char* direction);
void protocol_print_stats(const protocol_handler_t* handler);

// Specific packet creators
uint16_t protocol_create_ping(uint8_t* buffer, uint16_t buffer_size, 
                             protocol_handler_t* handler);
uint16_t protocol_create_sensor_data(float temperature, float humidity, 
                                    float pressure, uint8_t* buffer, 
                                    uint16_t buffer_size, protocol_handler_t* handler);
uint16_t protocol_create_status(uint32_t uptime, float cpu_usage, 
                               uint8_t task_count, uint8_t* buffer,
                               uint16_t buffer_size, protocol_handler_t* handler);
uint16_t protocol_create_error(protocol_error_t error, uint8_t* buffer,
                              uint16_t buffer_size, protocol_handler_t* handler);

#endif