#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdbool.h>
#include <time.h>

typedef enum {
    TEST_PASS,
    TEST_FAIL,
    TEST_SKIP,
    TEST_ERROR
} TestResult;

typedef struct {
    const char *name;
    TestResult result;
    double execution_time;
    const char *error_message;
} TestCase;

typedef struct {
    const char *suite_name;
    TestCase *tests;
    int test_count;
    int passed;
    int failed;
    int skipped;
    int errors;
    double total_time;
} TestSuite;

typedef struct {
    TestSuite *suites;
    int suite_count;
    int total_tests;
    int total_passed;
    int total_failed;
    int total_skipped;
    int total_errors;
    double total_execution_time;
    time_t start_time;
    time_t end_time;
} TestRunner;

// Test framework functions
TestRunner* createTestRunner(void);
TestSuite* createTestSuite(const char *name);
void addTestCase(TestSuite *suite, const char *name, TestResult (*test_func)(void), double (*test_func_timer)(void));
void runTestSuite(TestSuite *suite);
void runAllTests(TestRunner *runner);
void printTestResults(const TestRunner *runner);
void generateTestReport(const TestRunner *runner, const char *filename);
void freeTestRunner(TestRunner *runner);
void freeTestSuite(TestSuite *suite);

// Assertion macros
#define ASSERT_TRUE(condition) \
    do { \
        if (!(condition)) { \
            return TEST_FAIL; \
        } \
    } while(0)

#define ASSERT_FALSE(condition) \
    do { \
        if (condition) { \
            return TEST_FAIL; \
        } \
    } while(0)

#define ASSERT_EQ(expected, actual) \
    do { \
        if ((expected) != (actual)) { \
            return TEST_FAIL; \
        } \
    } while(0)

#define ASSERT_NE(expected, actual) \
    do { \
        if ((expected) == (actual)) { \
            return TEST_FAIL; \
        } \
    } while(0)

#define ASSERT_STR_EQ(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) != 0) { \
            return TEST_FAIL; \
        } \
    } while(0)

#define ASSERT_STR_NE(expected, actual) \
    do { \
        if (strcmp((expected), (actual)) == 0) { \
            return TEST_FAIL; \
        } \
    } while(0)

#define ASSERT_NULL(ptr) \
    do { \
        if ((ptr) != NULL) { \
            return TEST_FAIL; \
        } \
    } while(0)

#define ASSERT_NOT_NULL(ptr) \
    do { \
        if ((ptr) == NULL) { \
            return TEST_FAIL; \
        } \
    } while(0)

#define TEST(name) TestResult test_##name(void)
#define TIMER_TEST(name) double timer_##name(void)

// Performance testing
typedef struct {
    const char *operation_name;
    int iterations;
    double total_time;
    double avg_time;
    double min_time;
    double max_time;
} PerformanceTest;

PerformanceTest* createPerformanceTest(const char *name, int iterations);
void runPerformanceTest(PerformanceTest *test, void (*func)(void));
void printPerformanceResults(const PerformanceTest *test);
void freePerformanceTest(PerformanceTest *test);

#endif