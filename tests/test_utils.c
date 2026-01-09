#include "test_config.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

// Test statistics
static unsigned int total_assertions = 0;
static unsigned int failed_assertions = 0;
static unsigned int current_failures = 0;

// Timer storage
#ifdef TEST_ENABLE_TIMING
typedef struct {
    char name[64];
    clock_t start_time;
    double total_time;
    unsigned int call_count;
} test_timer_t;

static test_timer_t timers[16];
static unsigned int timer_count = 0;
#endif

// Memory checking
#ifdef TEST_ENABLE_MEMORY_CHECK
static size_t initial_memory_usage = 0;

#ifdef _WIN32
#include <psapi.h>
static size_t get_current_memory_usage(void) {
    PROCESS_MEMORY_COUNTERS pmc;
    if (GetProcessMemoryInfo(GetCurrentProcess(), &pmc, sizeof(pmc))) {
        return pmc.WorkingSetSize;
    }
    return 0;
}
#else
// Linux memory check would go here
static size_t get_current_memory_usage(void) {
    return 0;
}
#endif
#endif

void test_record_failure(const char* file, int line, const char* expression) {
    total_assertions++;
    failed_assertions++;
    current_failures++;
    
    const char* filename = strrchr(file, '/');
    if (!filename) filename = file;
    else filename++;
    
    printf(TEST_COLOR_RED "  ASSERTION FAILED: %s:%d\n" TEST_COLOR_RESET, filename, line);
    printf("  Expression: %s\n", expression);
    
    if (current_failures >= TEST_MAX_ASSERTION_FAILURES) {
        printf(TEST_COLOR_RED "  Too many failures, aborting test\n" TEST_COLOR_RESET);
        exit(1);
    }
}

void test_init_suite(test_suite_t* suite, const char* name) {
    memset(suite, 0, sizeof(test_suite_t));
    suite->name = name;
    suite->tests = NULL;
    suite->test_count = 0;
}

void test_add_case(test_suite_t* suite, const char* name, 
                   test_result_t (*func)(void), const char* description) {
    // Simple implementation - would use dynamic allocation in real project
    static test_case_t test_cases[50];
    
    if (suite->test_count < 50) {
        test_cases[suite->test_count].name = name;
        test_cases[suite->test_count].func = func;
        test_cases[suite->test_count].description = description;
        test_cases[suite->test_count].timeout_ms = 5000;
        
        suite->tests = test_cases;
        suite->test_count++;
    }
}

test_result_t test_run_suite(test_suite_t* suite) {
    printf("\n%sRunning Test Suite: %s%s\n", 
           TEST_COLOR_CYAN, suite->name, TEST_COLOR_RESET);
    printf("================================================\n");
    
    unsigned int passed = 0;
    unsigned int failed = 0;
    unsigned int skipped = 0;
    unsigned int errors = 0;
    
    for (unsigned int i = 0; i < suite->test_count; i++) {
        test_case_t* test = &suite->tests[i];
        
        printf("\nTest %u/%u: %s\n", i + 1, suite->test_count, test->name);
        if (test->description) {
            printf("  Description: %s\n", test->description);
        }
        
        current_failures = 0;
        
        #ifdef TEST_ENABLE_TIMING
        test_start_timer(test->name);
        #endif
        
        test_result_t result = test->func();
        
        #ifdef TEST_ENABLE_TIMING
        double elapsed = test_stop_timer(test->name);
        printf("  Time: %.2f ms\n", elapsed);
        #endif
        
        switch (result) {
            case TEST_PASS:
                printf(TEST_COLOR_GREEN "  ✓ PASS" TEST_COLOR_RESET "\n");
                passed++;
                break;
                
            case TEST_FAIL:
                printf(TEST_COLOR_RED "  ✗ FAIL" TEST_COLOR_RESET "\n");
                failed++;
                break;
                
            case TEST_SKIP:
                printf(TEST_COLOR_YELLOW "  ⚠ SKIP" TEST_COLOR_RESET "\n");
                skipped++;
                break;
                
            case TEST_ERROR:
                printf(TEST_COLOR_MAGENTA "  ⚡ ERROR" TEST_COLOR_RESET "\n");
                errors++;
                break;
        }
    }
    
    printf("\n================================================\n");
    printf("Suite Summary: %s\n", suite->name);
    printf("  Total:  %u\n", suite->test_count);
    printf("  Passed: %u\n", passed);
    printf("  Failed: %u\n", failed);
    printf("  Skipped:%u\n", skipped);
    printf("  Errors: %u\n", errors);
    printf("  Assertions: %u total, %u failed\n", total_assertions, failed_assertions);
    
    if (failed == 0 && errors == 0) {
        printf(TEST_COLOR_GREEN "  ✓ ALL TESTS PASSED" TEST_COLOR_RESET "\n");
        return TEST_PASS;
    } else {
        printf(TEST_COLOR_RED "  ✗ SOME TESTS FAILED" TEST_COLOR_RESET "\n");
        return TEST_FAIL;
    }
}

void test_print_summary(const test_suite_t* suite) {
    // Already printed in test_run_suite
    (void)suite;
}

#ifdef TEST_ENABLE_MEMORY_CHECK
void test_start_memory_check(void) {
    initial_memory_usage = get_current_memory_usage();
    printf("Memory check started: %zu bytes\n", initial_memory_usage);
}

size_t test_end_memory_check(void) {
    size_t final_memory_usage = get_current_memory_usage();
    size_t delta = final_memory_usage - initial_memory_usage;
    
    printf("Memory check ended:\n");
    printf("  Initial: %zu bytes\n", initial_memory_usage);
    printf("  Final:   %zu bytes\n", final_memory_usage);
    printf("  Delta:   %+zu bytes\n", delta);
    
    if (delta > 1024) {
        printf(TEST_COLOR_YELLOW "  Warning: Possible memory leak detected\n" TEST_COLOR_RESET);
    }
    
    return delta;
}
#endif

#ifdef TEST_ENABLE_TIMING
void test_start_timer(const char* name) {
    if (timer_count >= 16) return;
    
    // Find existing timer or create new one
    int index = -1;
    for (unsigned int i = 0; i < timer_count; i++) {
        if (strcmp(timers[i].name, name) == 0) {
            index = i;
            break;
        }
    }
    
    if (index == -1) {
        index = timer_count++;
        strncpy(timers[index].name, name, 63);
        timers[index].name[63] = '\0';
        timers[index].total_time = 0.0;
        timers[index].call_count = 0;
    }
    
    timers[index].start_time = clock();
}

double test_stop_timer(const char* name) {
    clock_t end_time = clock();
    
    for (unsigned int i = 0; i < timer_count; i++) {
        if (strcmp(timers[i].name, name) == 0) {
            double elapsed = (double)(end_time - timers[i].start_time) * 1000.0 / CLOCKS_PER_SEC;
            timers[i].total_time += elapsed;
            timers[i].call_count++;
            return elapsed;
        }
    }
    
    return 0.0;
}

void test_print_timers(void) {
    if (timer_count == 0) return;
    
    printf("\nPerformance Timers:\n");
    printf("----------------------------------------\n");
    
    for (unsigned int i = 0; i < timer_count; i++) {
        double avg_time = timers[i].call_count > 0 ? 
                         timers[i].total_time / timers[i].call_count : 0.0;
        
        printf("  %-20s: %6.2f ms total, %5.1f ms avg (%u calls)\n",
               timers[i].name, timers[i].total_time, avg_time, timers[i].call_count);
    }
}
#endif