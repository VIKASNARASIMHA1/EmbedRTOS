#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

// Test framework configuration
#define TEST_MAX_ASSERTION_FAILURES  10
#define TEST_TIMEOUT_SECONDS         30
#define TEST_ENABLE_COLOR_OUTPUT     1
#define TEST_ENABLE_TIMING           1
#define TEST_ENABLE_MEMORY_CHECK     1

// Test result codes
typedef enum {
    TEST_PASS = 0,
    TEST_FAIL = 1,
    TEST_SKIP = 2,
    TEST_ERROR = 3
} test_result_t;

// Test case structure
typedef struct {
    const char* name;
    test_result_t (*func)(void);
    const char* description;
    unsigned int timeout_ms;
} test_case_t;

// Test suite structure
typedef struct {
    const char* name;
    test_case_t* tests;
    unsigned int test_count;
    unsigned int setup_called : 1;
    unsigned int teardown_called : 1;
} test_suite_t;

// Assertion macros
#define TEST_ASSERT(condition) \
    do { \
        if (!(condition)) { \
            test_record_failure(__FILE__, __LINE__, #condition); \
            return TEST_FAIL; \
        } \
    } while(0)

#define TEST_ASSERT_EQUAL(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            test_record_failure(__FILE__, __LINE__, #expected " == " #actual); \
            printf("  Expected: %ld\n", (long)(expected)); \
            printf("  Actual:   %ld\n", (long)(actual)); \
            return TEST_FAIL; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_EQUAL(not_expected, actual) \
    do { \
        if ((not_expected) == (actual)) { \
            test_record_failure(__FILE__, __LINE__, #not_expected " != " #actual); \
            return TEST_FAIL; \
        } \
    } while(0)

#define TEST_ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != NULL) { \
            test_record_failure(__FILE__, __LINE__, #ptr " == NULL"); \
            return TEST_FAIL; \
        } \
    } while(0)

#define TEST_ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            test_record_failure(__FILE__, __LINE__, #ptr " != NULL"); \
            return TEST_FAIL; \
        } \
    } while(0)

#define TEST_ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            test_record_failure(__FILE__, __LINE__, #condition " is true"); \
            return TEST_FAIL; \
        } \
    } while(0)

#define TEST_ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            test_record_failure(__FILE__, __LINE__, #condition " is false"); \
            return TEST_FAIL; \
        } \
    } while(0)

#define TEST_ASSERT_STRING_EQUAL(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            test_record_failure(__FILE__, __LINE__, #expected " == " #actual); \
            printf("  Expected: \"%s\"\n", expected); \
            printf("  Actual:   \"%s\"\n", actual); \
            return TEST_FAIL; \
        } \
    } while(0)

// Test management functions
void test_init_suite(test_suite_t* suite, const char* name);
void test_add_case(test_suite_t* suite, const char* name, 
                   test_result_t (*func)(void), const char* description);
test_result_t test_run_suite(test_suite_t* suite);
void test_print_summary(const test_suite_t* suite);

// Failure recording
void test_record_failure(const char* file, int line, const char* expression);

// Memory checking (for detecting leaks in tests)
#ifdef TEST_ENABLE_MEMORY_CHECK
void test_start_memory_check(void);
size_t test_end_memory_check(void);
#endif

// Timing functions
#ifdef TEST_ENABLE_TIMING
void test_start_timer(const char* name);
double test_stop_timer(const char* name);
void test_print_timers(void);
#endif

// Color output
#ifdef TEST_ENABLE_COLOR_OUTPUT
#define TEST_COLOR_RED     "\033[31m"
#define TEST_COLOR_GREEN   "\033[32m"
#define TEST_COLOR_YELLOW  "\033[33m"
#define TEST_COLOR_BLUE    "\033[34m"
#define TEST_COLOR_MAGENTA "\033[35m"
#define TEST_COLOR_CYAN    "\033[36m"
#define TEST_COLOR_RESET   "\033[0m"
#else
#define TEST_COLOR_RED     ""
#define TEST_COLOR_GREEN   ""
#define TEST_COLOR_YELLOW  ""
#define TEST_COLOR_BLUE    ""
#define TEST_COLOR_MAGENTA ""
#define TEST_COLOR_CYAN    ""
#define TEST_COLOR_RESET   ""
#endif

#endif // TEST_CONFIG_H