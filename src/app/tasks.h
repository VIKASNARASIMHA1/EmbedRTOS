#ifndef TASKS_H
#define TASKS_H

// Task function prototypes
void sensor_task(void* arg);
void comm_task(void* arg);
void monitor_task(void* arg);

// Task configurations
#define SENSOR_TASK_PRIORITY    1
#define SENSOR_TASK_PERIOD_MS   10

#define COMM_TASK_PRIORITY      2
#define COMM_TASK_PERIOD_MS     50

#define MONITOR_TASK_PRIORITY   3
#define MONITOR_TASK_PERIOD_MS  1000

// Shared data structures
typedef struct {
    float temperature;
    float humidity;
    float pressure;
    uint32_t sample_count;
    bool data_ready;
} sensor_data_t;

typedef struct {
    uint32_t packets_sent;
    uint32_t packets_received;
    uint32_t errors;
    bool connected;
} comm_status_t;

typedef struct {
    uint32_t uptime_seconds;
    float cpu_usage;
    uint32_t task_count;
    uint32_t memory_used;
    uint32_t memory_total;
} system_status_t;

// Global data accessors
sensor_data_t* get_sensor_data(void);
comm_status_t* get_comm_status(void);
system_status_t* get_system_status(void);

#endif