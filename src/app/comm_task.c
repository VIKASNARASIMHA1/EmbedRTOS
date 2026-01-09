#include "tasks.h"
#include "../kernel/scheduler.h"
#include "../hal/hal.h"
#include "../protocols/comm_protocol.h"
#include "../utils/logger.h"
#include <string.h>

// Global communication status
static comm_status_t comm_status;
static protocol_handler_t protocol_handler;

// Simulated UART
static uart_t comm_uart;

// Buffer for incoming/outgoing data
static uint8_t rx_buffer[256];
static uint8_t tx_buffer[256];

// Process incoming command
static void process_command(const protocol_packet_t* packet) {
    if (!packet) return;
    
    LOG_DEBUG("Processing command: 0x%02X (%s)", 
              packet->command, protocol_command_to_string(packet->command));
    
    switch (packet->command) {
        case CMD_PING: {
            LOG_INFO("Received PING command");
            // Send PONG response
            uint16_t tx_len = protocol_create_packet(CMD_PONG, NULL, 0, 
                                                    tx_buffer, sizeof(tx_buffer), 
                                                    &protocol_handler);
            if (tx_len > 0) {
                hal_uart_send(&comm_uart, tx_buffer, tx_len);
                comm_status.packets_sent++;
            }
            break;
        }
        
        case CMD_GET_SENSOR_DATA: {
            LOG_INFO("Received GET_SENSOR_DATA command");
            sensor_data_t* sensor_data = get_sensor_data();
            
            if (sensor_data && sensor_data->data_ready) {
                uint8_t sensor_data_bytes[12];
                uint8_t* temp_ptr = (uint8_t*)&sensor_data->temperature;
                uint8_t* hum_ptr = (uint8_t*)&sensor_data->humidity;
                uint8_t* press_ptr = (uint8_t*)&sensor_data->pressure;
                
                memcpy(&sensor_data_bytes[0], temp_ptr, 4);
                memcpy(&sensor_data_bytes[4], hum_ptr, 4);
                memcpy(&sensor_data_bytes[8], press_ptr, 4);
                
                uint16_t tx_len = protocol_create_packet(CMD_SENSOR_DATA, 
                                                        sensor_data_bytes, 12,
                                                        tx_buffer, sizeof(tx_buffer),
                                                        &protocol_handler);
                if (tx_len > 0) {
                    hal_uart_send(&comm_uart, tx_buffer, tx_len);
                    comm_status.packets_sent++;
                }
            }
            break;
        }
        
        case CMD_GET_STATUS: {
            LOG_INFO("Received GET_STATUS command");
            
            uint32_t uptime = scheduler_get_tick_count() / 1000; // Convert to seconds
            float cpu_usage = scheduler_get_cpu_usage();
            uint8_t task_count = scheduler.task_count;
            
            uint8_t status_data[9];
            status_data[0] = (uptime >> 24) & 0xFF;
            status_data[1] = (uptime >> 16) & 0xFF;
            status_data[2] = (uptime >> 8) & 0xFF;
            status_data[3] = uptime & 0xFF;
            
            uint8_t* cpu_ptr = (uint8_t*)&cpu_usage;
            memcpy(&status_data[4], cpu_ptr, 4);
            status_data[8] = task_count;
            
            uint16_t tx_len = protocol_create_packet(CMD_STATUS_RESPONSE,
                                                    status_data, sizeof(status_data),
                                                    tx_buffer, sizeof(tx_buffer),
                                                    &protocol_handler);
            if (tx_len > 0) {
                hal_uart_send(&comm_uart, tx_buffer, tx_len);
                comm_status.packets_sent++;
            }
            break;
        }
        
        case CMD_START_OTA: {
            LOG_INFO("Received START_OTA command");
            // In a real system, this would start OTA process
            uint8_t response[] = {0x01}; // ACK
            uint16_t tx_len = protocol_create_packet(CMD_OTA_COMPLETE,
                                                    response, sizeof(response),
                                                    tx_buffer, sizeof(tx_buffer),
                                                    &protocol_handler);
            if (tx_len > 0) {
                hal_uart_send(&comm_uart, tx_buffer, tx_len);
                comm_status.packets_sent++;
            }
            break;
        }
        
        default: {
            LOG_WARN("Unknown command: 0x%02X", packet->command);
            uint8_t error = ERROR_INVALID_CMD;
            uint16_t tx_len = protocol_create_packet(CMD_ERROR,
                                                    &error, 1,
                                                    tx_buffer, sizeof(tx_buffer),
                                                    &protocol_handler);
            if (tx_len > 0) {
                hal_uart_send(&comm_uart, tx_buffer, tx_len);
                comm_status.packets_sent++;
            }
            break;
        }
    }
}

void comm_task(void* arg) {
    (void)arg;
    
    LOG_INFO("Communication task starting...");
    
    // Initialize communication status
    memset(&comm_status, 0, sizeof(comm_status));
    comm_status.connected = true;
    
    // Initialize protocol handler
    protocol_init(&protocol_handler);
    
    // Initialize simulated UART
    hal_uart_init(&comm_uart, 115200);
    
    uint32_t last_activity_check = 0;
    uint32_t last_stats_print = 0;
    
    while (1) {
        // Check for incoming data
        if (hal_uart_available(&comm_uart)) {
            uint32_t rx_len = hal_uart_receive(&comm_uart, rx_buffer, sizeof(rx_buffer));
            
            if (rx_len > 0) {
                comm_status.packets_received++;
                
                // Parse the packet
                protocol_packet_t packet;
                if (protocol_parse_packet(rx_buffer, rx_len, &packet, &protocol_handler)) {
                    process_command(&packet);
                } else {
                    comm_status.errors++;
                    LOG_WARN("Failed to parse packet (errors: %u)", comm_status.errors);
                }
            }
        }
        
        // Send periodic status update
        uint32_t ticks = scheduler_get_tick_count();
        if (ticks - last_activity_check >= 10000) { // Every 10 seconds
            // Simulate sending a heartbeat
            uint16_t tx_len = protocol_create_ping(tx_buffer, sizeof(tx_buffer), 
                                                  &protocol_handler);
            if (tx_len > 0) {
                hal_uart_send(&comm_uart, tx_buffer, tx_len);
                comm_status.packets_sent++;
                
                LOG_DEBUG("Sent heartbeat (total packets sent: %u)", 
                         comm_status.packets_sent);
            }
            
            last_activity_check = ticks;
        }
        
        // Print statistics every 30 seconds
        if (ticks - last_stats_print >= 30000) {
            LOG_INFO("Comm stats: Sent=%u, Received=%u, Errors=%u, Connected=%s",
                    comm_status.packets_sent, comm_status.packets_received,
                    comm_status.errors, comm_status.connected ? "YES" : "NO");
            
            protocol_print_stats(&protocol_handler);
            last_stats_print = ticks;
        }
        
        scheduler_delay(COMM_TASK_PERIOD_MS);
    }
}

// Getter for communication status
comm_status_t* get_comm_status(void) {
    return &comm_status;
}