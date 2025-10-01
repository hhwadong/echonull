#include "../include/contact_manager.h"
#include "../include/memory_allocator.h"
#include "../include/security.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

static int testsRun = 0;
static int testsPassed = 0;

#define ASSERT(condition, message) \
    do { \
        testsRun++; \
        if (condition) { \
            testsPassed++; \
            printf("✓ %s\n", message); \
        } else { \
            printf("✗ %s\n", message); \
        } \
    } while (0)

void testContactOperations(void) {
    printf("\n=== Testing Contact Operations ===\n");
    ContactNode *contacts = NULL;

    Contact test1 = {"John Doe", "1234567890", "john@example.com"};
    Contact test2 = {"Jane Smith", "9876543210", "jane@example.com"};

    addContact(&contacts, &test1);
    ASSERT(contacts != NULL, "Add first contact");
    ASSERT(strcmp(contacts->contact.name, "John Doe") == 0, "Contact name matches");

    addContact(&contacts, &test2);
    ASSERT(strcmp(contacts->contact.name, "Jane Smith") == 0, "Second contact name matches (head insertion)");

    Contact updated = {"John Updated", "1111111111", "john@updated.com"};
    bool updateResult = updateContact(contacts, "John Doe", &updated);
    ASSERT(updateResult == true, "Update existing contact");

    ContactNode *current = contacts;
    while (current != NULL && strcmp(current->contact.name, "John Updated") != 0) {
        current = current->next;
    }
    ASSERT(current != NULL, "Updated contact found in list");

    bool updateNonExistent = updateContact(contacts, "Nonexistent", &updated);
    ASSERT(updateNonExistent == false, "Update nonexistent contact fails");

    bool deleteResult = deleteContact(&contacts, "Jane Smith");
    ASSERT(deleteResult == true, "Delete existing contact");
    ASSERT(contacts->next == NULL, "Contact removed from list");

    bool deleteNonExistent = deleteContact(&contacts, "Nonexistent");
    ASSERT(deleteNonExistent == false, "Delete nonexistent contact fails");

    freeContacts(&contacts);
    ASSERT(contacts == NULL, "Free all contacts");
}

void testMemoryAllocator(void) {
    printf("\n=== Testing Memory Allocator ===\n");
    initializeMemory();

    void *ptr1 = myMalloc(100);
    ASSERT(ptr1 != NULL, "Allocate 100 bytes");

    void *ptr2 = myMalloc(200);
    ASSERT(ptr2 != NULL, "Allocate 200 bytes");

    void *ptr3 = myMalloc(4096);
    ASSERT(ptr3 == NULL, "Allocate too much memory fails");

    myFree(ptr1);
    myFree(ptr2);

    void *ptr4 = myMalloc(300);
    ASSERT(ptr4 != NULL, "Allocate after coalescing");

    myFree(ptr4);

    size_t totalFree, largestBlock;
    int fragmentCount;
    analyzeMemory(&totalFree, &largestBlock, &fragmentCount);
    ASSERT(totalFree > 0, "Memory analysis shows free space");
    ASSERT(largestBlock > 0, "Largest block detected");
}

void testSecurity(void) {
    printf("\n=== Testing Security Functions ===\n");
    char original[] = "Hello, World!";
    char encrypted[100];
    char decrypted[100];

    encryptData(original, encrypted, strlen(original));
    ASSERT(memcmp(original, encrypted, strlen(original)) != 0, "Encryption changes data");

    decryptData(encrypted, decrypted, strlen(original));
    ASSERT(memcmp(original, decrypted, strlen(original)) == 0, "Decryption restores data");

    char empty[] = "";
    char encryptedEmpty[1];
    char decryptedEmpty[1];

    encryptData(empty, encryptedEmpty, 0);
    decryptData(encryptedEmpty, decryptedEmpty, 0);
    ASSERT(decryptedEmpty[0] == '\0', "Empty string encryption/decryption");
}

void testFileOperations(void) {
    printf("\n=== Testing File Operations ===\n");
    ContactNode *contacts = NULL;

    Contact test1 = {"Alice", "1234567890", "alice@test.com"};
    Contact test2 = {"Bob", "0987654321", "bob@test.com"};

    addContact(&contacts, &test1);
    addContact(&contacts, &test2);

    saveContacts(contacts, "test_contacts.dat");
    ASSERT(remove("test_contacts.dat") == 0, "Save contacts creates file");

    ContactNode *loadedContacts = NULL;
    loadContacts(&loadedContacts, "test_contacts.dat");
    ASSERT(loadedContacts == NULL, "Load non-existent file creates empty list");

    freeContacts(&contacts);
}

void runAllTests(void) {
    printf("=== EchoNull Unit Tests ===\n");

    testContactOperations();
    testMemoryAllocator();
    testSecurity();
    testFileOperations();

    printf("\n=== Test Results ===\n");
    printf("Tests run: %d\n", testsRun);
    printf("Tests passed: %d\n", testsPassed);
    printf("Success rate: %.1f%%\n", testsRun > 0 ? (float)testsPassed / testsRun * 100 : 0);

    if (testsPassed == testsRun) {
        printf("🎉 All tests passed!\n");
    } else {
        printf("❌ Some tests failed.\n");
    }
}

int main(void) {
    runAllTests();
    return (testsPassed == testsRun) ? 0 : 1;
}