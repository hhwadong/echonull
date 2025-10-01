#ifndef CONTACT_MANAGER_H
#define CONTACT_MANAGER_H

#include <stdbool.h>

typedef struct {
    char name[50];
    char phone[20];
    char email[50];
} Contact;

typedef struct ContactNode {
    Contact contact;
    struct ContactNode *next;
} ContactNode;

void addContact(ContactNode **head, const Contact *contact);
void displayContacts(const ContactNode *head);
bool updateContact(ContactNode *head, const char *name, const Contact *newContact);
bool deleteContact(ContactNode **head, const char *name);
void saveContacts(const ContactNode *head, const char *filename);
void loadContacts(ContactNode **head, const char *filename);
void freeContacts(ContactNode **head);

#endif