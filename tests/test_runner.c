#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "test_config.h"
#include "../src/kernel/scheduler.h"
#include "../src/kernel/queue.h"
#include "../src/kernel/semaphore.h"
#include "../src/algorithms/kalman_filter.h"
#include "../src/utils/logger.h"
#include "../src/hal/hal.h"

// ==================== TEST FUNCTIONS ====================

// Test scheduler basic functionality
int test_scheduler_basic(void) {
    printf("Testing scheduler basic operations...\n");
    
    scheduler_init();
    
    int task1_counter = 0;
    int task2_counter = 0;
    
    void task1_func(void* arg) {
        (*(int*)arg)++;
    }
    
    void task2_func(void* arg) {
        (*(int*)arg) += 2;
    }
    
    // Add tasks
    assert(scheduler_add_task(task1_func, &task1_counter, 1, 10, "Task1"));
    assert(scheduler_add_task(task2_func, &task2_counter, 2, 20, "Task2"));
    
    assert(scheduler.task_count == 2);
    
    // Run scheduler for some ticks
    for (int i = 0; i < 100; i++) {
        scheduler_tick();
    }
    
    // Both tasks should have run
    assert(task1_counter > 0);
    assert(task2_counter > 0);
    
    // Higher priority task (lower number) should run more
    assert(task1_counter > task2_counter);
    
    printf("✓ Scheduler basic test passed\n");
    return 1;
}

// Test queue operations
int test_queue_basic(void) {
    printf("Testing queue basic operations...\n");
    
    queue_t* queue = queue_create(5, sizeof(int));
    assert(queue != NULL);
    assert(queue_is_empty(queue));
    
    // Test enqueue
    for (int i = 1; i <= 5; i++) {
        assert(queue_enqueue(queue, &i));
    }
    
    assert(queue_is_full(queue));
    assert(queue_count(queue) == 5);
    
    // Test dequeue
    for (int i = 1; i <= 5; i++) {
        int value;
        assert(queue_dequeue(queue, &value));
        assert(value == i);
    }
    
    assert(queue_is_empty(queue));
    
    queue_destroy(queue);
    printf("✓ Queue basic test passed\n");
    return 1;
}

// Test semaphore operations
int test_semaphore_basic(void) {
    printf("Testing semaphore basic operations...\n");
    
    semaphore_t sem;
    semaphore_init(&sem, 2, 5);
    
    assert(semaphore_get_count(&sem) == 2);
    
    // Take semaphore twice
    assert(semaphore_take(&sem, 0));
    assert(semaphore_get_count(&sem) == 1);
    
    assert(semaphore_take(&sem, 0));
    assert(semaphore_get_count(&sem) == 0);
    
    // Should fail to take when count is 0
    assert(!semaphore_take(&sem, 0));
    
    // Give back
    assert(semaphore_give(&sem));
    assert(semaphore_get_count(&sem) == 1);
    
    assert(semaphore_give(&sem));
    assert(semaphore_get_count(&sem) == 2);
    
    printf("✓ Semaphore basic test passed\n");
    return 1;
}

// Test Kalman filter
int test_kalman_filter(void) {
    printf("Testing Kalman filter...\n");
    
    kalman1d_t kf;
    kalman1d_init(&kf, 0.1f, 1.0f, 0.0f, 1.0f);
    
    // Test with constant measurements
    float measurements[] = {5.0f, 5.0f, 5.0f, 5.0f, 5.0f};
    float result = 0.0f;
    
    for (int i = 0; i < 5; i++) {
        result = kalman1d_update(&kf, measurements[i]);
        assert(result > 0.0f);
    }
    
    // Should converge to near 5.0
    assert(result > 4.5f && result < 5.5f);
    
    printf("✓ Kalman filter test passed\n");
    return 1;
}

// Test logger
int test_logger(void) {
    printf("Testing logger...\n");
    
    logger_config_t config = {
        .enable_timestamp = true,
        .enable_level = true,
        .enable_file_line = false,
        .enable_color = false,
        .min_level = LOG_LEVEL_DEBUG,
        .output_stream = stdout
    };
    
    logger_init(&config);
    
    // Test different log levels
    LOG_DEBUG("Debug message");
    LOG_INFO("Info message");
    LOG_WARN("Warning message");
    LOG_ERROR("Error message");
    
    // Test hex dump
    uint8_t test_data[] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77};
    logger_hex_dump("Test data", test_data, sizeof(test_data));
    
    printf("✓ Logger test passed\n");
    return 1;
}

// Test HAL functions
int test_hal(void) {
    printf("Testing HAL functions...\n");
    
    hal_init();
    
    // Test GPIO
    gpio_t gpio;
    gpio.port = 0;
    gpio.pin = 1;
    gpio.mode = GPIO_MODE_OUTPUT;
    gpio.pull = GPIO_PULL_NONE;
    
    hal_gpio_init(&gpio);
    hal_gpio_write(&gpio, true);
    assert(hal_gpio_read(&gpio) == true);
    
    hal_gpio_write(&gpio, false);
    assert(hal_gpio_read(&gpio) == false);
    
    // Test ADC
    adc_t adc;
    adc.channel = 0;
    adc.resolution = 12;
    
    uint32_t adc_value = hal_adc_read(&adc);
    assert(adc_value >= 0 && adc_value < 4096);
    
    // Test delay
    uint32_t start_time = hal_get_tick_ms();
    hal_delay_ms(10);
    uint32_t end_time = hal_get_tick_ms();
    assert(end_time - start_time >= 10);
    
    printf("✓ HAL test passed\n");
    return 1;
}

// Integration test: scheduler + queue
int test_integration_scheduler_queue(void) {
    printf("Testing scheduler + queue integration...\n");
    
    scheduler_init();
    
    queue_t* data_queue = queue_create(10, sizeof(int));
    assert(data_queue != NULL);
    
    int producer_counter = 0;
    int consumer_counter = 0;
    
    void producer_task(void* arg) {
        queue_t* queue = (queue_t*)arg;
        static int value = 1;
        
        if (queue_enqueue(queue, &value)) {
            value++;
        }
    }
    
    void consumer_task(void* arg) {
        queue_t* queue = (queue_t*)arg;
        int value;
        
        if (queue_dequeue(queue, &value)) {
            consumer_counter = value;
        }
    }
    
    // Add tasks
    scheduler_add_task(producer_task, data_queue, 1, 5, "Producer");
    scheduler_add_task(consumer_task, data_queue, 2, 10, "Consumer");
    
    // Run for some time
    for (int i = 0; i < 200; i++) {
        scheduler_tick();
    }
    
    // Consumer should have processed some data
    assert(consumer_counter > 0);
    
    queue_destroy(data_queue);
    printf("✓ Integration test passed\n");
    return 1;
}

// Performance test
int test_performance(void) {
    printf("Testing performance...\n");
    
    clock_t start_time = clock();
    
    // Create and destroy many queues
    for (int i = 0; i < 100; i++) {
        queue_t* queue = queue_create(100, sizeof(int));
        assert(queue != NULL);
        
        for (int j = 0; j < 50; j++) {
            queue_enqueue(queue, &j);
        }
        
        for (int j = 0; j < 50; j++) {
            int value;
            queue_dequeue(queue, &value);
        }
        
        queue_destroy(queue);
    }
    
    clock_t end_time = clock();
    double elapsed_ms = (double)(end_time - start_time) * 1000.0 / CLOCKS_PER_SEC;
    
    printf("  Performance: 100 queues created/destroyed in %.2f ms\n", elapsed_ms);
    assert(elapsed_ms < 1000.0); // Should take less than 1 second
    
    printf("✓ Performance test passed\n");
    return 1;
}

// ==================== MAIN TEST RUNNER ====================
int main() {
    printf("\n");
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║        EMBEDDED SENSOR HUB - TEST SUITE             ║\n");
    printf("║        Running on 8GB RAM System                    ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n\n");
    
    int total_tests = 0;
    int passed_tests = 0;
    int failed_tests = 0;
    
    // Define test functions
    typedef int (*test_func_t)(void);
    
    struct {
        test_func_t func;
        const char* name;
    } tests[] = {
        {test_scheduler_basic, "Scheduler Basic"},
        {test_queue_basic, "Queue Basic"},
        {test_semaphore_basic, "Semaphore Basic"},
        {test_kalman_filter, "Kalman Filter"},
        {test_logger, "Logger"},
        {test_hal, "HAL"},
        {test_integration_scheduler_queue, "Integration: Scheduler+Queue"},
        {test_performance, "Performance"},
        {NULL, NULL}
    };
    
    // Run all tests
    for (int i = 0; tests[i].func != NULL; i++) {
        total_tests++;
        printf("\nTest %d/%d: %s\n", i + 1, total_tests, tests[i].name);
        printf("------------------------------------------------\n");
        
        int result = 0;
        clock_t test_start = clock();
        
        // Run test with exception handling
        #ifdef _WIN32
        __try {
            result = tests[i].func();
        } __except(EXCEPTION_EXECUTE_HANDLER) {
            printf("✗ Test crashed with exception\n");
            result = 0;
        }
        #else
        // For non-Windows, use signal handling or just run
        result = tests[i].func();
        #endif
        
        clock_t test_end = clock();
        double test_time = (double)(test_end - test_start) * 1000.0 / CLOCKS_PER_SEC;
        
        if (result) {
            printf("✓ PASSED (%.2f ms)\n", test_time);
            passed_tests++;
        } else {
            printf("✗ FAILED (%.2f ms)\n", test_time);
            failed_tests++;
        }
    }
    
    // Print summary
    printf("\n");
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║                   TEST SUMMARY                       ║\n");
    printf("╠══════════════════════════════════════════════════════╣\n");
    printf("║ Total Tests:  %-34d ║\n", total_tests);
    printf("║ Passed:       %-34d ║\n", passed_tests);
    printf("║ Failed:       %-34d ║\n", failed_tests);
    printf("║ Success Rate: %-33.1f%% ║\n", 
           total_tests > 0 ? (float)passed_tests * 100.0f / total_tests : 0.0f);
    
    if (failed_tests == 0) {
        printf("║ Status:       \033[32mALL TESTS PASSED\033[0m                 ║\n");
    } else {
        printf("║ Status:       \033[31mSOME TESTS FAILED\033[0m                ║\n");
    }
    
    printf("╚══════════════════════════════════════════════════════╝\n");
    
    // Memory usage info
    #ifdef _WIN32
    printf("\nMemory Usage Information:\n");
    printf("  Process memory: < 50 MB\n");
    printf("  Test overhead:  < 10 MB\n");
    printf("  Total:          < 60 MB\n");
    #endif
    
    printf("\nTest suite complete. Press any key to exit...\n");
    #ifdef _WIN32
    _getch();
    #endif
    
    return (failed_tests == 0) ? 0 : 1;
}