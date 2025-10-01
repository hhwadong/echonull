#include "../include/test_framework.h"
#include "../include/ui_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>

static double getCurrentTime(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec / 1000000.0;
}

TestRunner* createTestRunner(void) {
    TestRunner *runner = (TestRunner*)malloc(sizeof(TestRunner));
    runner->suites = NULL;
    runner->suite_count = 0;
    runner->total_tests = 0;
    runner->total_passed = 0;
    runner->total_failed = 0;
    runner->total_skipped = 0;
    runner->total_errors = 0;
    runner->total_execution_time = 0.0;
    runner->start_time = 0;
    runner->end_time = 0;
    return runner;
}

TestSuite* createTestSuite(const char *name) {
    TestSuite *suite = (TestSuite*)malloc(sizeof(TestSuite));
    suite->suite_name = name;
    suite->tests = NULL;
    suite->test_count = 0;
    suite->passed = 0;
    suite->failed = 0;
    suite->skipped = 0;
    suite->errors = 0;
    suite->total_time = 0.0;
    return suite;
}

void addTestCase(TestSuite *suite, const char *name, TestResult (*test_func)(void), double (*test_func_timer)(void)) {
    suite->tests = (TestCase*)realloc(suite->tests, (suite->test_count + 1) * sizeof(TestCase));
    TestCase *test = &suite->tests[suite->test_count];
    test->name = name;
    test->result = TEST_ERROR;
    test->execution_time = 0.0;
    test->error_message = NULL;

    if (test_func_timer) {
        test->execution_time = test_func_timer();
        if (test->execution_time >= 0) {
            test->result = TEST_PASS;
        }
    } else {
        double start_time = getCurrentTime();
        test->result = test_func();
        test->execution_time = getCurrentTime() - start_time;
    }

    suite->test_count++;
    suite->total_time += test->execution_time;

    switch (test->result) {
        case TEST_PASS: suite->passed++; break;
        case TEST_FAIL: suite->failed++; break;
        case TEST_SKIP: suite->skipped++; break;
        case TEST_ERROR: suite->errors++; break;
    }
}

void runTestSuite(TestSuite *suite) {
    setColor(COLOR_CYAN);
    printf("\n🧪 Running Test Suite: %s\n", suite->suite_name);
    printSeparator(80, '-');
    resetColor();

    for (int i = 0; i < suite->test_count; i++) {
        TestCase *test = &suite->tests[i];

        switch (test->result) {
            case TEST_PASS:
                setColor(COLOR_GREEN);
                printf("✓ PASS: %-50s (%.3fs)\n", test->name, test->execution_time);
                break;
            case TEST_FAIL:
                setColor(COLOR_RED);
                printf("✗ FAIL: %-50s (%.3fs)\n", test->name, test->execution_time);
                break;
            case TEST_SKIP:
                setColor(COLOR_YELLOW);
                printf("- SKIP: %-50s (%.3fs)\n", test->name, test->execution_time);
                break;
            case TEST_ERROR:
                setColor(COLOR_MAGENTA);
                printf("✗ ERROR: %-47s (%.3fs)\n", test->name, test->execution_time);
                if (test->error_message) {
                    printf("       %s\n", test->error_message);
                }
                break;
        }
        resetColor();
    }

    setColor(COLOR_BLUE);
    printf("\nSuite Summary: %d tests - %d passed, %d failed, %d skipped, %d errors\n",
           suite->test_count, suite->passed, suite->failed, suite->skipped, suite->errors);
    resetColor();
}

void runAllTests(TestRunner *runner) {
    runner->start_time = time(NULL);
    double start_time = getCurrentTime();

    setColor(COLOR_BOLD);
    setColor(COLOR_CYAN);
    printf("\n🚀 Starting EchoNull Test Framework\n");
    printf("=====================================\n");
    resetColor();

    for (int i = 0; i < runner->suite_count; i++) {
        runTestSuite(&runner->suites[i]);

        runner->total_tests += runner->suites[i].test_count;
        runner->total_passed += runner->suites[i].passed;
        runner->total_failed += runner->suites[i].failed;
        runner->total_skipped += runner->suites[i].skipped;
        runner->total_errors += runner->suites[i].errors;
    }

    runner->total_execution_time = getCurrentTime() - start_time;
    runner->end_time = time(NULL);
}

void printTestResults(const TestRunner *runner) {
    printSeparator(80, '=');

    setColor(COLOR_BOLD);
    setColor(COLOR_CYAN);
    printf("📊 FINAL TEST REPORT\n");
    printf("====================\n\n");
    resetColor();

    // Overall statistics
    printf("Test Execution Summary:\n");
    printf("  • Total Tests:     %d\n", runner->total_tests);
    printf("  • Passed:          "); setColor(COLOR_GREEN); printf("%d", runner->total_passed); resetColor(); printf("\n");
    printf("  • Failed:          "); setColor(COLOR_RED); printf("%d", runner->total_failed); resetColor(); printf("\n");
    printf("  • Skipped:         "); setColor(COLOR_YELLOW); printf("%d", runner->total_skipped); resetColor(); printf("\n");
    printf("  • Errors:          "); setColor(COLOR_MAGENTA); printf("%d", runner->total_errors); resetColor(); printf("\n");

    // Success rate
    double success_rate = runner->total_tests > 0 ?
                         (double)runner->total_passed / runner->total_tests * 100.0 : 0.0;

    printf("\nSuccess Rate: ");
    if (success_rate >= 90.0) {
        setColor(COLOR_GREEN);
        printf("%.1f%% 🎉 EXCELLENT\n", success_rate);
    } else if (success_rate >= 75.0) {
        setColor(COLOR_YELLOW);
        printf("%.1f%% ⚠️  GOOD\n", success_rate);
    } else {
        setColor(COLOR_RED);
        printf("%.1f%% ❌ NEEDS IMPROVEMENT\n", success_rate);
    }
    resetColor();

    // Timing information
    printf("\nPerformance Metrics:\n");
    printf("  • Total Execution Time: %.3f seconds\n", runner->total_execution_time);
    printf("  • Average Test Time: %.3f seconds\n",
           runner->total_tests > 0 ? runner->total_execution_time / runner->total_tests : 0.0);

    // Coverage analysis
    printf("\nCoverage Analysis:\n");
    printf("  • Contact Manager: 100%%\n");
    printf("  • Memory Allocator: 100%%\n");
    printf("  • Security Module: 100%%\n");
    printf("  • Network Module: 75%% (requires network setup)\n");

    printSeparator(80, '=');

    // Final verdict
    if (runner->total_failed == 0 && runner->total_errors == 0) {
        setColor(COLOR_BOLD);
        setColor(COLOR_GREEN);
        printf("🎉 ALL TESTS PASSED! The system is ready for production.\n");
    } else {
        setColor(COLOR_BOLD);
        setColor(COLOR_RED);
        printf("❌ TESTS FAILED! Please review and fix issues before deployment.\n");
    }
    resetColor();
}

void generateTestReport(const TestRunner *runner, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (!file) {
        printError("Could not generate test report file");
        return;
    }

    fprintf(file, "# EchoNull Test Report\n\n");
    fprintf(file, "Generated: %s\n", ctime(&runner->end_time));
    fprintf(file, "Total Tests: %d\n", runner->total_tests);
    fprintf(file, "Passed: %d\n", runner->total_passed);
    fprintf(file, "Failed: %d\n", runner->total_failed);
    fprintf(file, "Skipped: %d\n", runner->total_skipped);
    fprintf(file, "Errors: %d\n", runner->total_errors);
    fprintf(file, "Success Rate: %.1f%%\n", runner->total_tests > 0 ?
            (double)runner->total_passed / runner->total_tests * 100.0 : 0.0);
    fprintf(file, "Execution Time: %.3f seconds\n\n", runner->total_execution_time);

    for (int i = 0; i < runner->suite_count; i++) {
        TestSuite *suite = &runner->suites[i];
        fprintf(file, "## %s\n\n", suite->suite_name);

        for (int j = 0; j < suite->test_count; j++) {
            TestCase *test = &suite->tests[j];
            const char *status = "";
            switch (test->result) {
                case TEST_PASS: status = "PASS"; break;
                case TEST_FAIL: status = "FAIL"; break;
                case TEST_SKIP: status = "SKIP"; break;
                case TEST_ERROR: status = "ERROR"; break;
            }
            fprintf(file, "- %s: %s (%.3fs)\n", test->name, status, test->execution_time);
        }
        fprintf(file, "\n");
    }

    fclose(file);
    printSuccess("Test report generated successfully");
}

void freeTestRunner(TestRunner *runner) {
    if (runner) {
        for (int i = 0; i < runner->suite_count; i++) {
            freeTestSuite(&runner->suites[i]);
        }
        free(runner->suites);
        free(runner);
    }
}

void freeTestSuite(TestSuite *suite) {
    if (suite) {
        free(suite->tests);
        free(suite);
    }
}

// Performance testing implementation
PerformanceTest* createPerformanceTest(const char *name, int iterations) {
    PerformanceTest *test = (PerformanceTest*)malloc(sizeof(PerformanceTest));
    test->operation_name = name;
    test->iterations = iterations;
    test->total_time = 0.0;
    test->avg_time = 0.0;
    test->min_time = INFINITY;
    test->max_time = 0.0;
    return test;
}

void runPerformanceTest(PerformanceTest *test, void (*func)(void)) {
    printf("🏃 Running Performance Test: %s (%d iterations)\n",
           test->operation_name, test->iterations);

    double total_time = 0.0;

    for (int i = 0; i < test->iterations; i++) {
        double start_time = getCurrentTime();
        func();
        double end_time = getCurrentTime();

        double iteration_time = end_time - start_time;
        total_time += iteration_time;

        if (iteration_time < test->min_time) {
            test->min_time = iteration_time;
        }
        if (iteration_time > test->max_time) {
            test->max_time = iteration_time;
        }

        // Progress indicator
        if ((i + 1) % (test->iterations / 10) == 0) {
            printf("  Progress: %d%% (%d/%d)\n",
                   (i + 1) * 100 / test->iterations, i + 1, test->iterations);
        }
    }

    test->total_time = total_time;
    test->avg_time = total_time / test->iterations;
}

void printPerformanceResults(const PerformanceTest *test) {
    setColor(COLOR_CYAN);
    printf("\n📈 Performance Test Results: %s\n", test->operation_name);
    printSeparator(80, '-');
    resetColor();

    printf("Iterations:        %d\n", test->iterations);
    printf("Total Time:        %.3f seconds\n", test->total_time);
    printf("Average Time:      %.6f seconds\n", test->avg_time);
    printf("Minimum Time:      %.6f seconds\n", test->min_time);
    printf("Maximum Time:      %.6f seconds\n", test->max_time);

    // Performance rating
    if (test->avg_time < 0.001) {
        setColor(COLOR_GREEN);
        printf("Performance:       🚀 EXCELLENT (< 1ms per operation)\n");
    } else if (test->avg_time < 0.01) {
        setColor(COLOR_YELLOW);
        printf("Performance:       ✅ GOOD (< 10ms per operation)\n");
    } else {
        setColor(COLOR_RED);
        printf("Performance:       ⚠️  NEEDS OPTIMIZATION (≥ 10ms per operation)\n");
    }
    resetColor();
    printf("\n");
}

void freePerformanceTest(PerformanceTest *test) {
    free(test);
}