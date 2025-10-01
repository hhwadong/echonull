#include "../include/contact_manager.h"
#include "../include/memory_allocator.h"
#include "../include/network_sync.h"
#include "../include/security.h"
#include "../include/ui_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define CONTACTS_FILE "contacts.dat"
#define DEFAULT_PORT 8080

static ContactNode *contacts = NULL;
static volatile bool running = true;

void signalHandler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        running = false;
        printf("\nShutting down gracefully...\n");
        stopServer();
    }
}

void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
    // Clear any potential error states
    if (ferror(stdin)) {
        clearerr(stdin);
    }
}

void printLogo(void) {
    int width = getTerminalWidth();
    if (width < 60) width = 60;

    setColor(COLOR_CYAN);
    printCentered("", width);
    printCentered("", width);
    printCentered("    ██╗    ██╗██╗██████╗ ██████╗ ██╗    ██╗███████╗██████╗ ", width);
    printCentered("    ██║    ██║██║██╔══██╗██╔══██╗██║    ██║██╔════╝██╔══██╗", width);
    printCentered("    ██║ █╗ ██║██║██████╔╝██║  ██║██║ █╗ ██║█████╗  ██████╔╝", width);
    printCentered("    ██║███╗██║██║██╔══██╗██║  ██║██║███╗██║██╔══╝  ██╔══██╗", width);
    printCentered("    ╚███╔███╔╝██║██║  ██║██████╔╝╚███╔███╔╝███████╗██║  ██║", width);
    printCentered("     ╚══╝╚══╝ ╚═╝╚═╝  ╚═╝╚═════╝  ╚══╝╚══╝ ╚══════╝╚═╝  ╚═╝", width);
    printCentered("", width);
    printCentered("", width);

    setColor(COLOR_BOLD);
    setColor(COLOR_GREEN);
    printCentered("█ E C H O N U L L   C O N T A C T   M A N A G E R █", width);
    resetColor();

    setColor(COLOR_YELLOW);
    printCentered("Secure • Fast • Reliable", width);
    resetColor();

    printSeparator(width, '=');
}

void printMenu(void) {
    clearScreen();
    printLogo();

    int width = getTerminalWidth();
    if (width < 60) width = 60;

    const char *menuItems[] = {
        "📝 Add Contact",
        "📋 Display All Contacts",
        "✏️  Update Contact",
        "🗑️  Delete Contact",
        "💾 Save Contacts",
        "📂 Load Contacts",
        "🌐 Start Server",
        "🔄 Sync with Server",
        "📊 Memory Analysis",
        "🔍 Visualize Memory",
        "🚪 Exit Program"
    };

    const char *descriptions[] = {
        "Create a new contact entry",
        "View all saved contacts",
        "Modify existing contact information",
        "Remove contact from database",
        "Persist contacts to disk",
        "Load contacts from storage",
        "Initialize network server",
        "Synchronize with remote server",
        "Show memory usage statistics",
        "Display memory visualization",
        "Exit the application"
    };

    for (int i = 0; i < 11; i++) {
        if (i == 10) {
            setColor(COLOR_RED);
            printf("\n    %2d. %s", 0, menuItems[i]);
        } else {
            setColor(COLOR_CYAN);
            printf("\n    %2d. %s", i + 1, menuItems[i]);
        }

        setColor(COLOR_GRAY);
        printf("\n        %s", descriptions[i]);
        resetColor();
    }

    printf("\n\n");
    setColor(COLOR_BOLD);
    setColor(COLOR_GREEN);
    printf("    ══════════════════════════════════════════════════════════════\n");
    printf("    Enter your choice [0-10]: ");
    resetColor();
}

void addContactMenu(void) {
    Contact newContact;

    printf("Enter name: ");
    if (fgets(newContact.name, sizeof(newContact.name), stdin) == NULL) {
        return;
    }
    newContact.name[strcspn(newContact.name, "\n")] = '\0';

    if (strlen(newContact.name) == 0) {
        printf("Contact name cannot be empty.\n");
        return;
    }

    printf("Enter phone: ");
    if (fgets(newContact.phone, sizeof(newContact.phone), stdin) == NULL) {
        return;
    }
    newContact.phone[strcspn(newContact.phone, "\n")] = '\0';

    printf("Enter email: ");
    if (fgets(newContact.email, sizeof(newContact.email), stdin) == NULL) {
        return;
    }
    newContact.email[strcspn(newContact.email, "\n")] = '\0';

    // Check if contact with same name already exists
    ContactNode *current = contacts;
    while (current != NULL) {
        if (strcmp(current->contact.name, newContact.name) == 0) {
            printf("A contact with name '%s' already exists.\n", newContact.name);
            return;
        }
        current = current->next;
    }

    addContact(&contacts, &newContact);
    printf("Contact '%s' added successfully!\n", newContact.name);
}

void updateContactMenu(void) {
    if (contacts == NULL) {
        printf("No contacts available to update.\n");
        return;
    }

    char input[50];
    Contact newContact;

    displayContacts(contacts);
    printf("Enter the NAME of contact to update: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        clearInputBuffer();
        printf("Error reading input. Update cancelled.\n");
        return;
    }
    input[strcspn(input, "\n")] = '\0';

    // Check if input is empty
    if (strlen(input) == 0) {
        printf("Invalid input. Please enter a contact name. Update cancelled.\n");
        return;
    }

    printf("Enter new name: ");
    if (fgets(newContact.name, sizeof(newContact.name), stdin) == NULL) {
        clearInputBuffer();
        printf("Error reading input. Update cancelled.\n");
        return;
    }
    newContact.name[strcspn(newContact.name, "\n")] = '\0';

    if (strlen(newContact.name) == 0) {
        printf("Contact name cannot be empty. Update cancelled.\n");
        return;
    }

    printf("Enter new phone: ");
    if (fgets(newContact.phone, sizeof(newContact.phone), stdin) == NULL) {
        clearInputBuffer();
        printf("Error reading input. Update cancelled.\n");
        return;
    }
    newContact.phone[strcspn(newContact.phone, "\n")] = '\0';

    printf("Enter new email: ");
    if (fgets(newContact.email, sizeof(newContact.email), stdin) == NULL) {
        clearInputBuffer();
        printf("Error reading input. Update cancelled.\n");
        return;
    }
    newContact.email[strcspn(newContact.email, "\n")] = '\0';

    if (updateContact(contacts, input, &newContact)) {
        printf("Contact updated successfully!\n");
    } else {
        printf("Contact '%s' not found.\n", input);
    }
}

void deleteContactMenu(void) {
    if (contacts == NULL) {
        printf("No contacts available to delete.\n");
        return;
    }

    char input[50];

    displayContacts(contacts);
    printf("Enter the NAME of contact to delete: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        return;
    }
    input[strcspn(input, "\n")] = '\0';

    // Check if input is empty
    if (strlen(input) == 0) {
        printf("Invalid input. Please enter a contact name.\n");
        return;
    }

    if (deleteContact(&contacts, input)) {
        printf("Contact '%s' deleted successfully!\n", input);
    } else {
        printf("Contact '%s' not found.\n", input);
    }
}

void startServerMenu(void) {
    int port;
    printf("Enter port number (default %d): ", DEFAULT_PORT);
    if (scanf("%d", &port) != 1) {
        port = DEFAULT_PORT;
        clearInputBuffer();
        printf("Using default port %d\n", port);
    } else {
        clearInputBuffer();
        if (port < 1024 || port > 65535) {
            printf("Invalid port number. Using default port %d\n", DEFAULT_PORT);
            port = DEFAULT_PORT;
        }
    }

    startServer(port);
}

void syncWithServerMenu(void) {
    char serverIP[16];
    int port;

    printf("Enter server IP: ");
    if (fgets(serverIP, sizeof(serverIP), stdin) == NULL) {
        return;
    }
    serverIP[strcspn(serverIP, "\n")] = '\0';

    if (strlen(serverIP) == 0) {
        printf("Server IP cannot be empty.\n");
        return;
    }

    printf("Enter port (default %d): ", DEFAULT_PORT);
    if (scanf("%d", &port) != 1) {
        port = DEFAULT_PORT;
        clearInputBuffer();
        printf("Using default port %d\n", port);
    } else {
        clearInputBuffer();
        if (port < 1024 || port > 65535) {
            printf("Invalid port number. Using default port %d\n", DEFAULT_PORT);
            port = DEFAULT_PORT;
        }
    }

    if (syncContacts(serverIP, port, &contacts)) {
        printf("Synchronization completed successfully!\n");
    } else {
        printf("Synchronization failed.\n");
    }
}

void memoryAnalysisMenu(void) {
    size_t totalFree, largestBlock;
    int fragmentCount;

    analyzeMemory(&totalFree, &largestBlock, &fragmentCount);

    printf("\n=== Memory Analysis ===\n");
    printf("Total free memory: %zu bytes\n", totalFree);
    printf("Largest free block: %zu bytes\n", largestBlock);
    printf("Number of fragments: %d\n", fragmentCount);

    if (fragmentCount > 1) {
        printf("Fragmentation level: %s\n",
               fragmentCount > 5 ? "High" : fragmentCount > 2 ? "Medium" : "Low");
    }
}

int main(void) {
    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    initializeMemory();
    loadContacts(&contacts, CONTACTS_FILE);

    printf("EchoNull Contact Manager started!\n");
    printf("Loaded %d contacts from %s\n",
           contacts ? 1 : 0, CONTACTS_FILE);

    int choice;
    while (running) {
        printMenu();

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number between 0 and 10.\n");
            clearInputBuffer();
            continue;
        }
        clearInputBuffer();

        if (choice < 0 || choice > 10) {
            printf("Invalid choice. Please enter a number between 0 and 10.\n");
            continue;
        }

        switch (choice) {
            case 1:
                addContactMenu();
                break;
            case 2:
                displayContacts(contacts);
                break;
            case 3:
                updateContactMenu();
                break;
            case 4:
                deleteContactMenu();
                break;
            case 5:
                saveContacts(contacts, CONTACTS_FILE);
                break;
            case 6:
                freeContacts(&contacts);
                loadContacts(&contacts, CONTACTS_FILE);
                printf("Contacts reloaded from file.\n");
                break;
            case 7:
                startServerMenu();
                break;
            case 8:
                syncWithServerMenu();
                break;
            case 9:
                memoryAnalysisMenu();
                break;
            case 10:
                visualizeMemory();
                break;
            case 0:
                running = false;
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    }

    saveContacts(contacts, CONTACTS_FILE);
    freeContacts(&contacts);
    stopServer();

    printf("Goodbye!\n");
    return 0;
}