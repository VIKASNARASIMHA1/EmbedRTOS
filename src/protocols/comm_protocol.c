#include "comm_protocol.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// CRC-16-CCITT polynomial: x^16 + x^12 + x^5 + 1 (0x1021)
static const uint16_t crc_table[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};

void protocol_init(protocol_handler_t* handler) {
    if (!handler) return;
    
    memset(handler, 0, sizeof(protocol_handler_t));
    handler->sequence_counter = 1; // Start from 1
    handler->initialized = true;
    
    printf("[PROTOCOL] Protocol handler initialized\n");
}

void protocol_deinit(protocol_handler_t* handler) {
    if (!handler) return;
    
    handler->initialized = false;
    printf("[PROTOCOL] Protocol handler deinitialized\n");
}

uint16_t protocol_calculate_crc(const uint8_t* data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < length; i++) {
        uint8_t index = (uint8_t)((crc >> 8) ^ data[i]);
        crc = (crc << 8) ^ crc_table[index];
    }
    
    return crc;
}

uint16_t protocol_create_packet(uint8_t command, const uint8_t* data, 
                               uint16_t data_len, uint8_t* buffer, 
                               uint16_t buffer_size, protocol_handler_t* handler) {
    if (!buffer || buffer_size < PROTOCOL_HEADER_SIZE + data_len + PROTOCOL_CRC_SIZE + 2) {
        printf("[PROTOCOL] Error: Buffer too small\n");
        return 0;
    }
    
    if (data_len > PROTOCOL_MAX_PACKET_SIZE) {
        printf("[PROTOCOL] Error: Data too large\n");
        return 0;
    }
    
    uint16_t packet_size = 0;
    uint16_t sequence = 0;
    
    // Get sequence number if handler is provided
    if (handler) {
        sequence = handler->sequence_counter++;
        handler->packets_sent++;
        handler->bytes_sent += PROTOCOL_HEADER_SIZE + data_len + PROTOCOL_CRC_SIZE + 2;
    }
    
    // Start byte
    buffer[packet_size++] = PROTOCOL_START_BYTE;
    
    // Command
    buffer[packet_size++] = command;
    
    // Sequence number (big endian)
    buffer[packet_size++] = (uint8_t)(sequence >> 8);
    buffer[packet_size++] = (uint8_t)(sequence & 0xFF);
    
    // Length (big endian)
    buffer[packet_size++] = (uint8_t)(data_len >> 8);
    buffer[packet_size++] = (uint8_t)(data_len & 0xFF);
    
    // Data
    if (data && data_len > 0) {
        memcpy(&buffer[packet_size], data, data_len);
        packet_size += data_len;
    }
    
    // Calculate CRC over header + data
    uint16_t crc = protocol_calculate_crc(&buffer[1], packet_size - 1);
    buffer[packet_size++] = (uint8_t)(crc >> 8);
    buffer[packet_size++] = (uint8_t)(crc & 0xFF);
    
    // End byte
    buffer[packet_size++] = PROTOCOL_END_BYTE;
    
    printf("[PROTOCOL] Created packet: CMD=0x%02X, SEQ=%u, Len=%u, CRC=0x%04X\n",
           command, sequence, data_len, crc);
    
    return packet_size;
}

bool protocol_parse_packet(const uint8_t* buffer, uint16_t buffer_len,
                          protocol_packet_t* packet, protocol_handler_t* handler) {
    if (!buffer || !packet || buffer_len < PROTOCOL_HEADER_SIZE + PROTOCOL_CRC_SIZE + 2) {
        printf("[PROTOCOL] Error: Invalid parameters\n");
        return false;
    }
    
    // Check start byte
    if (buffer[0] != PROTOCOL_START_BYTE) {
        printf("[PROTOCOL] Error: Invalid start byte: 0x%02X\n", buffer[0]);
        return false;
    }
    
    // Check end byte
    if (buffer[buffer_len - 1] != PROTOCOL_END_BYTE) {
        printf("[PROTOCOL] Error: Invalid end byte: 0x%02X\n", buffer[buffer_len - 1]);
        return false;
    }
    
    // Extract header
    packet->start_byte = buffer[0];
    packet->command = buffer[1];
    packet->sequence = (buffer[2] << 8) | buffer[3];
    packet->length = (buffer[4] << 8) | buffer[5];
    
    // Validate length
    uint16_t expected_packet_size = PROTOCOL_HEADER_SIZE + packet->length + PROTOCOL_CRC_SIZE + 1;
    if (buffer_len != expected_packet_size) {
        printf("[PROTOCOL] Error: Length mismatch. Expected %u, got %u\n",
               expected_packet_size, buffer_len);
        if (handler) handler->crc_errors++;
        return false;
    }
    
    // Extract data
    if (packet->length > 0 && packet->length <= PROTOCOL_MAX_PACKET_SIZE) {
        memcpy(packet->data, &buffer[6], packet->length);
    }
    
    // Extract and verify CRC
    uint16_t received_crc = (buffer[6 + packet->length] << 8) | buffer[7 + packet->length];
    uint16_t calculated_crc = protocol_calculate_crc(&buffer[1], 5 + packet->length);
    
    packet->crc = received_crc;
    packet->end_byte = buffer[buffer_len - 1];
    
    if (received_crc != calculated_crc) {
        printf("[PROTOCOL] Error: CRC mismatch. Received: 0x%04X, Calculated: 0x%04X\n",
               received_crc, calculated_crc);
        if (handler) handler->crc_errors++;
        return false;
    }
    
    // Update statistics
    if (handler) {
        handler->packets_received++;
        handler->bytes_received += buffer_len;
    }
    
    printf("[PROTOCOL] Parsed packet: CMD=0x%02X, SEQ=%u, Len=%u, CRC=0x%04X ✓\n",
           packet->command, packet->sequence, packet->length, packet->crc);
    
    return true;
}

const char* protocol_command_to_string(uint8_t command) {
    switch (command) {
        case CMD_PING: return "PING";
        case CMD_PONG: return "PONG";
        case CMD_GET_SENSOR_DATA: return "GET_SENSOR_DATA";
        case CMD_SENSOR_DATA: return "SENSOR_DATA";
        case CMD_SET_CONFIG: return "SET_CONFIG";
        case CMD_CONFIG_RESPONSE: return "CONFIG_RESPONSE";
        case CMD_START_OTA: return "START_OTA";
        case CMD_OTA_DATA: return "OTA_DATA";
        case CMD_OTA_COMPLETE: return "OTA_COMPLETE";
        case CMD_GET_STATUS: return "GET_STATUS";
        case CMD_STATUS_RESPONSE: return "STATUS_RESPONSE";
        case CMD_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

const char* protocol_error_to_string(uint8_t error) {
    switch (error) {
        case ERROR_NONE: return "NONE";
        case ERROR_INVALID_CMD: return "INVALID_CMD";
        case ERROR_INVALID_LEN: return "INVALID_LEN";
        case ERROR_CRC_MISMATCH: return "CRC_MISMATCH";
        case ERROR_BUSY: return "BUSY";
        case ERROR_OTA_FAILED: return "OTA_FAILED";
        case ERROR_SENSOR_FAILED: return "SENSOR_FAILED";
        default: return "UNKNOWN_ERROR";
    }
}

void protocol_print_packet(const protocol_packet_t* packet, const char* direction) {
    if (!packet) return;
    
    printf("[PROTOCOL] %s Packet:\n", direction);
    printf("  Command: 0x%02X (%s)\n", packet->command, 
           protocol_command_to_string(packet->command));
    printf("  Sequence: %u\n", packet->sequence);
    printf("  Length: %u\n", packet->length);
    printf("  CRC: 0x%04X\n", packet->crc);
    
    if (packet->length > 0) {
        printf("  Data: ");
        for (uint16_t i = 0; i < packet->length && i < 32; i++) {
            printf("%02X ", packet->data[i]);
        }
        if (packet->length > 32) printf("...");
        printf("\n");
        
        // Print ASCII if it looks like text
        if (packet->length <= 64) {
            printf("  ASCII: ");
            for (uint16_t i = 0; i < packet->length; i++) {
                if (packet->data[i] >= 32 && packet->data[i] <= 126) {
                    printf("%c", packet->data[i]);
                } else {
                    printf(".");
                }
            }
            printf("\n");
        }
    }
}

void protocol_print_stats(const protocol_handler_t* handler) {
    if (!handler) return;
    
    printf("\n╔══════════════════════════════════════════════════════╗\n");
    printf("║               PROTOCOL STATISTICS                   ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║ Packets Sent:     %-32u ║\n", handler->packets_sent);
    printf("║ Packets Received: %-32u ║\n", handler->packets_received);
    printf("║ Bytes Sent:       %-32u ║\n", handler->bytes_sent);
    printf("║ Bytes Received:   %-32u ║\n", handler->bytes_received);
    printf("║ CRC Errors:       %-32u ║\n", handler->crc_errors);
    printf("║ Timeout Errors:   %-32u ║\n", handler->timeout_errors);
    printf("║ Sequence Counter: %-32u ║\n", handler->sequence_counter);
    printf("╚══════════════════════════════════════════════════════╝\n");
}

// Specific packet creators
uint16_t protocol_create_ping(uint8_t* buffer, uint16_t buffer_size, 
                             protocol_handler_t* handler) {
    uint8_t ping_data[] = {0x50, 0x49, 0x4E, 0x47}; // "PING"
    return protocol_create_packet(CMD_PING, ping_data, sizeof(ping_data), 
                                 buffer, buffer_size, handler);
}

uint16_t protocol_create_sensor_data(float temperature, float humidity, 
                                    float pressure, uint8_t* buffer, 
                                    uint16_t buffer_size, protocol_handler_t* handler) {
    uint8_t data[12];
    
    // Pack floats into bytes
    uint8_t* temp_ptr = (uint8_t*)&temperature;
    uint8_t* hum_ptr = (uint8_t*)&humidity;
    uint8_t* press_ptr = (uint8_t*)&pressure;
    
    for (int i = 0; i < 4; i++) {
        data[i] = temp_ptr[i];
        data[i + 4] = hum_ptr[i];
        data[i + 8] = press_ptr[i];
    }
    
    return protocol_create_packet(CMD_SENSOR_DATA, data, sizeof(data), 
                                 buffer, buffer_size, handler);
}

uint16_t protocol_create_status(uint32_t uptime, float cpu_usage, 
                               uint8_t task_count, uint8_t* buffer,
                               uint16_t buffer_size, protocol_handler_t* handler) {
    uint8_t data[9];
    
    // Pack uptime (4 bytes), cpu_usage (4 bytes), task_count (1 byte)
    data[0] = (uptime >> 24) & 0xFF;
    data[1] = (uptime >> 16) & 0xFF;
    data[2] = (uptime >> 8) & 0xFF;
    data[3] = uptime & 0xFF;
    
    uint8_t* cpu_ptr = (uint8_t*)&cpu_usage;
    for (int i = 0; i < 4; i++) {
        data[4 + i] = cpu_ptr[i];
    }
    
    data[8] = task_count;
    
    return protocol_create_packet(CMD_STATUS_RESPONSE, data, sizeof(data), 
                                 buffer, buffer_size, handler);
}

uint16_t protocol_create_error(protocol_error_t error, uint8_t* buffer,
                              uint16_t buffer_size, protocol_handler_t* handler) {
    uint8_t data[1] = {error};
    return protocol_create_packet(CMD_ERROR, data, sizeof(data), 
                                 buffer, buffer_size, handler);
}