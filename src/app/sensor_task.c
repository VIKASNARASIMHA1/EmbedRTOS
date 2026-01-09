#include "tasks.h"
#include "../kernel/scheduler.h"
#include "../algorithms/kalman_filter.h"
#include "../utils/logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

// Global sensor data
static sensor_data_t sensor_data;
static kalman1d_t temp_kf;
static kalman1d_t hum_kf;
static kalman1d_t press_kf;

// Initialize random seed
static void init_random(void) {
    static bool initialized = false;
    if (!initialized) {
        srand((unsigned int)time(NULL));
        initialized = true;
    }
}

// Generate simulated sensor reading with drift and noise
static float simulate_temperature(void) {
    static float base_temp = 25.0f;
    static float drift = 0.0f;
    
    // Slowly drift temperature
    drift += ((float)rand() / RAND_MAX - 0.5f) * 0.01f;
    if (drift > 2.0f) drift = 2.0f;
    if (drift < -2.0f) drift = -2.0f;
    
    // Add noise
    float noise = ((float)rand() / RAND_MAX - 0.5f) * 0.5f;
    
    // Simulate day/night cycle
    uint32_t ticks = scheduler_get_tick_count();
    float time_of_day = sinf((float)ticks * 0.0001f);
    float daily_variation = time_of_day * 5.0f;
    
    return base_temp + drift + noise + daily_variation;
}

static float simulate_humidity(void) {
    static float base_hum = 50.0f;
    
    // Humidity correlates inversely with temperature
    float temp = sensor_data.temperature;
    float temp_factor = (temp - 25.0f) * -1.0f;
    
    // Add noise
    float noise = ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
    
    // Simulate humidity changes
    uint32_t ticks = scheduler_get_tick_count();
    float humidity_variation = sinf((float)ticks * 0.00005f) * 10.0f;
    
    return base_hum + temp_factor + noise + humidity_variation;
}

static float simulate_pressure(void) {
    static float base_pressure = 1013.25f;
    
    // Pressure changes slowly
    static float trend = 0.0f;
    trend += ((float)rand() / RAND_MAX - 0.5f) * 0.005f;
    if (trend > 10.0f) trend = 10.0f;
    if (trend < -10.0f) trend = -10.0f;
    
    // Add noise
    float noise = ((float)rand() / RAND_MAX - 0.5f) * 0.2f;
    
    return base_pressure + trend + noise;
}

void sensor_task(void* arg) {
    (void)arg;
    
    LOG_INFO("Sensor task starting...");
    
    init_random();
    
    // Initialize Kalman filters
    kalman1d_init(&temp_kf, 0.001f, 0.1f, 25.0f, 1.0f);
    kalman1d_init(&hum_kf, 0.001f, 0.5f, 50.0f, 1.0f);
    kalman1d_init(&press_kf, 0.01f, 1.0f, 1013.25f, 1.0f);
    
    // Initialize sensor data
    memset(&sensor_data, 0, sizeof(sensor_data));
    sensor_data.temperature = 25.0f;
    sensor_data.humidity = 50.0f;
    sensor_data.pressure = 1013.25f;
    sensor_data.data_ready = false;
    
    uint32_t last_print = 0;
    
    while (1) {
        // Generate raw sensor readings
        float raw_temp = simulate_temperature();
        float raw_hum = simulate_humidity();
        float raw_press = simulate_pressure();
        
        // Apply Kalman filtering
        sensor_data.temperature = kalman1d_update(&temp_kf, raw_temp);
        sensor_data.humidity = kalman1d_update(&hum_kf, raw_hum);
        sensor_data.pressure = kalman1d_update(&press_kf, raw_press);
        
        sensor_data.sample_count++;
        sensor_data.data_ready = true;
        
        // Print sensor data every 100 samples
        if (sensor_data.sample_count % 100 == 0) {
            uint32_t ticks = scheduler_get_tick_count();
            if (ticks - last_print >= 5000) {
                LOG_INFO("Sensor: Temp=%.2f°C, Hum=%.1f%%, Press=%.2fhPa, Samples=%u",
                        sensor_data.temperature, sensor_data.humidity, 
                        sensor_data.pressure, sensor_data.sample_count);
                last_print = ticks;
            }
        }
        
        // Simulate sensor processing delay
        scheduler_delay(SENSOR_TASK_PERIOD_MS);
    }
}

// Getter for sensor data
sensor_data_t* get_sensor_data(void) {
    return &sensor_data;
}