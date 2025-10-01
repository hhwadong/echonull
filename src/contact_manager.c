#include "../include/contact_manager.h"
#include "../include/ui_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

void addContact(ContactNode **head, const Contact *contact) {
    ContactNode *newNode = (ContactNode *)malloc(sizeof(ContactNode));
    if (newNode == NULL) {
        perror("Failed to allocate memory for new contact");
        return;
    }

    newNode->contact = *contact;
    newNode->next = *head;
    *head = newNode;
}

void displayContacts(const ContactNode *head) {
    if (head == NULL) {
        printf("\n");
        setColor(COLOR_YELLOW);
        printf("    📭 No contacts found in the database.\n");
        resetColor();
        return;
    }

    printf("\n");
    setColor(COLOR_BOLD);
    setColor(COLOR_CYAN);
    printf("    ┌─────────────────────────────────────────────────────────────────┐\n");
    printf("    │                        📋 CONTACT LIST                        │\n");
    printf("    └─────────────────────────────────────────────────────────────────┘\n\n");
    resetColor();

    const ContactNode *current = head;
    int count = 1;

    while (current != NULL) {
        setColor(COLOR_GREEN);
        printf("    ╔═══════════════════════════════════════════════════════════════╗\n");
        printf("    ║                     📇 CONTACT #%d                          ║\n", count);
        printf("    ╠═══════════════════════════════════════════════════════════════╣\n");
        resetColor();

        setColor(COLOR_WHITE);
        printf("    ║  👤 Name:    %-45s ║\n", current->contact.name);
        printf("    ║  📞 Phone:   %-45s ║\n", current->contact.phone);
        printf("    ║  📧 Email:   %-45s ║\n", current->contact.email);
        setColor(COLOR_GREEN);
        printf("    ╚═══════════════════════════════════════════════════════════════╝\n\n");
        resetColor();

        current = current->next;
        count++;
    }

    setColor(COLOR_BLUE);
    printf("    📊 Total Contacts: %d\n", count - 1);
    resetColor();
}

bool updateContact(ContactNode *head, const char *name, const Contact *newContact) {
    ContactNode *current = head;

    while (current != NULL) {
        if (strcmp(current->contact.name, name) == 0) {
            current->contact = *newContact;
            return true;
        }
        current = current->next;
    }

    return false;
}

bool deleteContact(ContactNode **head, const char *name) {
    if (*head == NULL) {
        return false;
    }

    ContactNode *current = *head;
    ContactNode *prev = NULL;

    while (current != NULL) {
        if (strcmp(current->contact.name, name) == 0) {
            if (prev == NULL) {
                *head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current);
            return true;
        }
        prev = current;
        current = current->next;
    }

    return false;
}

void saveContacts(const ContactNode *head, const char *filename) {
    FILE *file = fopen(filename, "wb");
    if (file == NULL) {
        perror("Failed to open file for saving");
        return;
    }

    const ContactNode *current = head;
    int count = 0;

    while (current != NULL) {
        if (fwrite(&current->contact, sizeof(Contact), 1, file) != 1) {
            perror("Failed to write contact to file");
            fclose(file);
            return;
        }
        count++;
        current = current->next;
    }

    fclose(file);
    printf("Saved %d contacts to %s\n", count, filename);
}

void loadContacts(ContactNode **head, const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (file == NULL) {
        printf("No existing contact file found. Starting with empty list.\n");
        return;
    }

    Contact tempContact;
    while (fread(&tempContact, sizeof(Contact), 1, file) == 1) {
        addContact(head, &tempContact);
    }

    fclose(file);
}

void freeContacts(ContactNode **head) {
    ContactNode *current = *head;
    ContactNode *next;

    while (current != NULL) {
        next = current->next;
        free(current);
        current = next;
    }

    *head = NULL;
}