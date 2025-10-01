#include "../include/test_framework.h"
#include "../include/contact_manager.h"
#include "../include/memory_allocator.h"
#include "../include/security.h"
#include "../include/ui_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// Contact Manager Tests
TEST(test_contact_add_single) {
    ContactNode *contacts = NULL;
    Contact test = {"John Doe", "1234567890", "john@example.com"};

    addContact(&contacts, &test);
    ASSERT_NOT_NULL(contacts);
    ASSERT_STR_EQ("John Doe", contacts->contact.name);

    freeContacts(&contacts);
    ASSERT_NULL(contacts);
    return TEST_PASS;
}

TEST(test_contact_add_multiple) {
    ContactNode *contacts = NULL;
    Contact contacts_data[3] = {
        {"Alice", "111", "alice@a.com"},
        {"Bob", "222", "bob@b.com"},
        {"Charlie", "333", "charlie@c.com"}
    };

    for (int i = 0; i < 3; i++) {
        addContact(&contacts, &contacts_data[i]);
    }

    ASSERT_NOT_NULL(contacts);
    ASSERT_STR_EQ("Charlie", contacts->contact.name); // Head insertion

    freeContacts(&contacts);
    return TEST_PASS;
}

TEST(test_contact_update_existing) {
    ContactNode *contacts = NULL;
    Contact original = {"Original", "123", "original@o.com"};
    Contact updated = {"Updated", "456", "updated@u.com"};

    addContact(&contacts, &original);
    bool result = updateContact(contacts, "Original", &updated);

    ASSERT_TRUE(result);
    ASSERT_STR_EQ("Updated", contacts->contact.name);

    freeContacts(&contacts);
    return TEST_PASS;
}

TEST(test_contact_update_nonexistent) {
    ContactNode *contacts = NULL;
    Contact updated = {"Updated", "456", "updated@u.com"};

    bool result = updateContact(contacts, "Nonexistent", &updated);
    ASSERT_FALSE(result);

    return TEST_PASS;
}

TEST(test_contact_delete_existing) {
    ContactNode *contacts = NULL;
    Contact test = {"ToDelete", "123", "delete@d.com"};

    addContact(&contacts, &test);
    ASSERT_NOT_NULL(contacts);

    bool result = deleteContact(&contacts, "ToDelete");
    ASSERT_TRUE(result);
    ASSERT_NULL(contacts);

    return TEST_PASS;
}

TEST(test_contact_delete_nonexistent) {
    ContactNode *contacts = NULL;

    bool result = deleteContact(&contacts, "Nonexistent");
    ASSERT_FALSE(result);

    return TEST_PASS;
}

TEST(test_contact_file_operations) {
    ContactNode *contacts = NULL;
    Contact test = {"FileTest", "123", "file@f.com"};

    addContact(&contacts, &test);
    saveContacts(contacts, "test_contacts.dat");

    ContactNode *loaded = NULL;
    loadContacts(&loaded, "test_contacts.dat");

    ASSERT_NOT_NULL(loaded);
    ASSERT_STR_EQ("FileTest", loaded->contact.name);

    freeContacts(&contacts);
    freeContacts(&loaded);
    remove("test_contacts.dat");

    return TEST_PASS;
}

// Memory Allocator Tests
TEST(test_memory_initialization) {
    initializeMemory();
    // Test that we can allocate after initialization
    void *ptr = myMalloc(100);
    ASSERT_NOT_NULL(ptr);
    myFree(ptr);
    return TEST_PASS;
}

TEST(test_memory_allocation_basic) {
    initializeMemory();

    void *ptr1 = myMalloc(100);
    void *ptr2 = myMalloc(200);

    ASSERT_NOT_NULL(ptr1);
    ASSERT_NOT_NULL(ptr2);
    ASSERT_NE(ptr1, ptr2);

    myFree(ptr1);
    myFree(ptr2);
    return TEST_PASS;
}

TEST(test_memory_allocation_large) {
    initializeMemory();

    void *ptr = myMalloc(3000);
    ASSERT_NOT_NULL(ptr);
    myFree(ptr);

    return TEST_PASS;
}

TEST(test_memory_allocation_too_large) {
    initializeMemory();

    void *ptr = myMalloc(10000);
    ASSERT_NULL(ptr);

    return TEST_PASS;
}

TEST(test_memory_fragmentation) {
    initializeMemory();

    // Create fragmentation
    void *ptr1 = myMalloc(1000);
    void *ptr2 = myMalloc(1000);
    void *ptr3 = myMalloc(1000);

    myFree(ptr2); // Free middle block

    // Try to allocate larger block that requires coalescing
    void *ptr4 = myMalloc(1500);
    ASSERT_NOT_NULL(ptr4);

    myFree(ptr1);
    myFree(ptr3);
    myFree(ptr4);

    return TEST_PASS;
}

// Security Tests
TEST(test_encryption_basic) {
    const char *original = "Hello, World!";
    char encrypted[100];
    char decrypted[100];

    encryptData(original, encrypted, strlen(original));
    decryptData(encrypted, decrypted, strlen(original));

    ASSERT_STR_EQ(original, decrypted);
    return TEST_PASS;
}

TEST(test_encryption_different) {
    const char *original = "Test message";
    char encrypted[100];

    encryptData(original, encrypted, strlen(original));
    ASSERT_NE(0, memcmp(original, encrypted, strlen(original)));
    return TEST_PASS;
}

TEST(test_encryption_empty) {
    const char *original = "";
    char encrypted[1];
    char decrypted[1];

    encryptData(original, encrypted, 0);
    decryptData(encrypted, decrypted, 0);

    return TEST_PASS;
}

// Integration Tests
TEST(test_full_contact_lifecycle) {
    ContactNode *contacts = NULL;

    // Add
    Contact contact = {"Integration", "1234567890", "integration@test.com"};
    addContact(&contacts, &contact);
    ASSERT_NOT_NULL(contacts);

    // Display (should not crash)
    displayContacts(contacts);

    // Update
    Contact updated = {"IntegrationUpdated", "0987654321", "updated@test.com"};
    bool update_result = updateContact(contacts, "Integration", &updated);
    ASSERT_TRUE(update_result);

    // Save and Load
    saveContacts(contacts, "integration_test.dat");
    ContactNode *loaded = NULL;
    loadContacts(&loaded, "integration_test.dat");

    ASSERT_NOT_NULL(loaded);
    ASSERT_STR_EQ("IntegrationUpdated", loaded->contact.name);

    // Delete
    bool delete_result = deleteContact(&loaded, "IntegrationUpdated");
    ASSERT_TRUE(delete_result);
    ASSERT_NULL(loaded);

    // Cleanup
    freeContacts(&contacts);
    remove("integration_test.dat");

    return TEST_PASS;
}

TEST(test_memory_stress) {
    initializeMemory();

    // Stress test with many small allocations
    void *pointers[100];
    for (int i = 0; i < 100; i++) {
        pointers[i] = myMalloc(32);
        ASSERT_NOT_NULL(pointers[i]);
    }

    // Free in random order
    for (int i = 0; i < 100; i += 3) {
        myFree(pointers[i]);
    }

    // Try to allocate larger block
    void *large_ptr = myMalloc(1000);
    ASSERT_NOT_NULL(large_ptr);

    // Clean up
    for (int i = 0; i < 100; i++) {
        if (i % 3 != 0) {
            myFree(pointers[i]);
        }
    }
    myFree(large_ptr);

    return TEST_PASS;
}

// Performance Tests
TIMER_TEST(timer_contact_add_1000) {
    ContactNode *contacts = NULL;
    Contact contact = {"PerfTest", "1234567890", "perf@test.com"};

    clock_t start = clock();
    for (int i = 0; i < 1000; i++) {
        addContact(&contacts, &contact);
    }
    clock_t end = clock();

    freeContacts(&contacts);
    return (double)(end - start) / CLOCKS_PER_SEC;
}

TIMER_TEST(timer_memory_alloc_free_10000) {
    initializeMemory();

    clock_t start = clock();
    for (int i = 0; i < 10000; i++) {
        void *ptr = myMalloc(64);
        myFree(ptr);
    }
    clock_t end = clock();

    return (double)(end - start) / CLOCKS_PER_SEC;
}

// Main test runner
int main(void) {
    clearScreen();

    // Create test runner
    TestRunner *runner = createTestRunner();

    // Contact Manager Test Suite
    TestSuite *contact_suite = createTestSuite("Contact Manager");
    addTestCase(contact_suite, "Add Single Contact", test_test_contact_add_single, NULL);
    addTestCase(contact_suite, "Add Multiple Contacts", test_test_contact_add_multiple, NULL);
    addTestCase(contact_suite, "Update Existing Contact", test_test_contact_update_existing, NULL);
    addTestCase(contact_suite, "Update Nonexistent Contact", test_test_contact_update_nonexistent, NULL);
    addTestCase(contact_suite, "Delete Existing Contact", test_test_contact_delete_existing, NULL);
    addTestCase(contact_suite, "Delete Nonexistent Contact", test_test_contact_delete_nonexistent, NULL);
    addTestCase(contact_suite, "File Operations", test_test_contact_file_operations, NULL);

    // Memory Allocator Test Suite
    TestSuite *memory_suite = createTestSuite("Memory Allocator");
    addTestCase(memory_suite, "Memory Initialization", test_test_memory_initialization, NULL);
    addTestCase(memory_suite, "Basic Allocation", test_test_memory_allocation_basic, NULL);
    addTestCase(memory_suite, "Large Allocation", test_test_memory_allocation_large, NULL);
    addTestCase(memory_suite, "Allocation Too Large", test_test_memory_allocation_too_large, NULL);
    addTestCase(memory_suite, "Memory Fragmentation", test_test_memory_fragmentation, NULL);

    // Security Test Suite
    TestSuite *security_suite = createTestSuite("Security Module");
    addTestCase(security_suite, "Basic Encryption", test_test_encryption_basic, NULL);
    addTestCase(security_suite, "Encryption Produces Different Output", test_test_encryption_different, NULL);
    addTestCase(security_suite, "Empty String Encryption", test_test_encryption_empty, NULL);

    // Integration Test Suite
    TestSuite *integration_suite = createTestSuite("Integration Tests");
    addTestCase(integration_suite, "Full Contact Lifecycle", test_test_full_contact_lifecycle, NULL);
    addTestCase(integration_suite, "Memory Stress Test", test_test_memory_stress, NULL);

    // Performance Test Suite
    TestSuite *performance_suite = createTestSuite("Performance Tests");
    addTestCase(performance_suite, "Add 1000 Contacts", NULL, timer_timer_contact_add_1000);
    addTestCase(performance_suite, "10000 Memory Alloc/Free Cycles", NULL, timer_timer_memory_alloc_free_10000);

    // Add suites to runner
    runner->suites = (TestSuite*)malloc(5 * sizeof(TestSuite));
    runner->suites[0] = *contact_suite;
    runner->suites[1] = *memory_suite;
    runner->suites[2] = *security_suite;
    runner->suites[3] = *integration_suite;
    runner->suites[4] = *performance_suite;
    runner->suite_count = 5;

    // Run all tests
    runAllTests(runner);
    printTestResults(runner);

    // Generate report
    generateTestReport(runner, "test_report.md");

    // Performance tests
    printf("\n");
    setColor(COLOR_BOLD);
    setColor(COLOR_CYAN);
    printf("🏃 Running Additional Performance Tests\n");
    printSeparator(80, '=');
    resetColor();

    // Performance test for contact operations
    PerformanceTest *contact_perf = createPerformanceTest("Contact Operations", 1000);
    runPerformanceTest(contact_perf, (void (*)(void))test_test_contact_add_single);
    printPerformanceResults(contact_perf);
    freePerformanceTest(contact_perf);

    // Memory allocation performance
    PerformanceTest *memory_perf = createPerformanceTest("Memory Allocation", 10000);
    runPerformanceTest(memory_perf, (void (*)(void))test_test_memory_allocation_basic);
    printPerformanceResults(memory_perf);
    freePerformanceTest(memory_perf);

    // Cleanup
    free(contact_suite);
    free(memory_suite);
    free(security_suite);
    free(integration_suite);
    free(performance_suite);
    freeTestRunner(runner);

    // Final verdict
    if (runner->total_failed == 0 && runner->total_errors == 0) {
        setColor(COLOR_BOLD);
        setColor(COLOR_GREEN);
        printf("\n🎉 COMPREHENSIVE TESTING COMPLETE - SYSTEM READY FOR PRODUCTION! 🎉\n");
        resetColor();
        return 0;
    } else {
        setColor(COLOR_BOLD);
        setColor(COLOR_RED);
        printf("\n❌ TEST FAILURES DETECTED - REVIEW REQUIRED BEFORE PRODUCTION ❌\n");
        resetColor();
        return 1;
    }
}