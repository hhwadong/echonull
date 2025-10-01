#include "../include/network_sync.h"
#include "../include/security.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 4096
#define DEFAULT_PORT 8080

static int serverSocket = -1;
static ClientInfo *clients = NULL;
static pthread_mutex_t clientsMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t serverThread;
static volatile bool serverRunning = false;

static void *acceptConnections(void *arg);
static void *clientHandler(void *arg);
static void broadcastToClients(const char *message, int senderSocket);
static void addClient(int socket, struct sockaddr_in address);
static void removeClient(int socket);
static void handleClientCommand(int clientSocket, const char *command, ContactNode **serverContacts);

void startServer(int port) {
    if (serverRunning) {
        printf("Server is already running on port %d\n", port);
        return;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        perror("Failed to create socket");
        return;
    }

    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("Failed to set socket options");
        close(serverSocket);
        return;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Failed to bind socket");
        close(serverSocket);
        return;
    }

    if (listen(serverSocket, MAX_CLIENTS) < 0) {
        perror("Failed to listen on socket");
        close(serverSocket);
        return;
    }

    serverRunning = true;
    printf("Server started on port %d\n", port);

    if (pthread_create(&serverThread, NULL, acceptConnections, NULL) != 0) {
        perror("Failed to create server thread");
        serverRunning = false;
        close(serverSocket);
        return;
    }

    pthread_detach(serverThread);
}

void *acceptConnections(void *arg) {
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);

    while (serverRunning) {
        int clientSocket = accept(serverSocket, (struct sockaddr *)&clientAddr, &clientLen);
        if (clientSocket < 0) {
            if (serverRunning) {
                perror("Failed to accept client connection");
            }
            continue;
        }

        printf("Client connected from %s:%d\n",
               inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port));

        addClient(clientSocket, clientAddr);

        pthread_t clientThread;
        if (pthread_create(&clientThread, NULL, clientHandler, &clientSocket) != 0) {
            perror("Failed to create client thread");
            removeClient(clientSocket);
            close(clientSocket);
        } else {
            pthread_detach(clientThread);
        }
    }

    return NULL;
}

void *clientHandler(void *arg) {
    int clientSocket = *(int *)arg;
    char buffer[BUFFER_SIZE];
    ContactNode *serverContacts = NULL;

    while (serverRunning) {
        ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead <= 0) {
            break;
        }

        buffer[bytesRead] = '\0';
        printf("Received from client: %s\n", buffer);

        char decrypted[BUFFER_SIZE];
        decryptData(buffer, decrypted, bytesRead);

        handleClientCommand(clientSocket, decrypted, &serverContacts);
    }

    removeClient(clientSocket);
    close(clientSocket);
    printf("Client disconnected\n");
    return NULL;
}

void handleClientCommand(int clientSocket, const char *command, ContactNode **serverContacts) {
    char response[BUFFER_SIZE];
    memset(response, 0, sizeof(response));

    if (strncmp(command, "ADD_CONTACT:", 12) == 0) {
        Contact newContact;
        if (sscanf(command + 12, "%49[^,],%19[^,],%49s",
                   newContact.name, newContact.phone, newContact.email) == 3) {
            addContact(serverContacts, &newContact);
            snprintf(response, sizeof(response), "Contact added: %s", newContact.name);
        } else {
            strcpy(response, "Invalid contact format");
        }
    } else if (strncmp(command, "GET_CONTACTS", 12) == 0) {
        char tempBuffer[BUFFER_SIZE - 100] = "";
        const ContactNode *current = *serverContacts;
        while (current != NULL) {
            char contactStr[200];
            snprintf(contactStr, sizeof(contactStr), "%s,%s,%s|",
                     current->contact.name, current->contact.phone, current->contact.email);
            strcat(tempBuffer, contactStr);
            current = current->next;
        }
        snprintf(response, sizeof(response), "CONTACTS:%s", tempBuffer);
    } else if (strncmp(command, "SYNC:", 5) == 0) {
        snprintf(response, sizeof(response), "SYNC_READY");
    } else {
        strcpy(response, "Unknown command");
    }

    char encryptedResponse[BUFFER_SIZE];
    encryptData(response, encryptedResponse, strlen(response));
    send(clientSocket, encryptedResponse, strlen(encryptedResponse), 0);
}

bool syncContacts(const char *serverIP, int port, ContactNode **localContacts) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Failed to create client socket");
        return false;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);

    if (inet_pton(AF_INET, serverIP, &serverAddr.sin_addr) <= 0) {
        perror("Invalid server IP address");
        close(sock);
        return false;
    }

    if (connect(sock, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
        perror("Failed to connect to server");
        close(sock);
        return false;
    }

    printf("Connected to server %s:%d\n", serverIP, port);

    char buffer[BUFFER_SIZE];
    strcpy(buffer, "SYNC:");
    char encrypted[BUFFER_SIZE];
    encryptData(buffer, encrypted, strlen(buffer));
    send(sock, encrypted, strlen(encrypted), 0);

    ssize_t bytesRead = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
        char decrypted[BUFFER_SIZE];
        decryptData(buffer, decrypted, bytesRead);

        if (strcmp(decrypted, "SYNC_READY") == 0) {
            strcpy(buffer, "GET_CONTACTS");
            encryptData(buffer, encrypted, strlen(buffer));
            send(sock, encrypted, strlen(encrypted), 0);

            bytesRead = recv(sock, buffer, BUFFER_SIZE - 1, 0);
            if (bytesRead > 0) {
                buffer[bytesRead] = '\0';
                decryptData(buffer, decrypted, bytesRead);

                if (strncmp(decrypted, "CONTACTS:", 9) == 0) {
                    freeContacts(localContacts);
                    char *contactsData = decrypted + 9;
                    char *contact = strtok(contactsData, "|");
                    while (contact != NULL) {
                        Contact newContact;
                        if (sscanf(contact, "%49[^,],%19[^,],%49s",
                                   newContact.name, newContact.phone, newContact.email) == 3) {
                            addContact(localContacts, &newContact);
                        }
                        contact = strtok(NULL, "|");
                    }
                }
            }
        }
    }

    close(sock);
    printf("Synchronization completed\n");
    return true;
}

void addClient(int socket, struct sockaddr_in address) {
    pthread_mutex_lock(&clientsMutex);

    ClientInfo *newClient = (ClientInfo *)malloc(sizeof(ClientInfo));
    newClient->socket = socket;
    newClient->address = address;
    newClient->next = clients;
    clients = newClient;

    pthread_mutex_unlock(&clientsMutex);
}

void removeClient(int socket) {
    pthread_mutex_lock(&clientsMutex);

    ClientInfo **current = &clients;
    while (*current != NULL) {
        if ((*current)->socket == socket) {
            ClientInfo *toRemove = *current;
            *current = (*current)->next;
            free(toRemove);
            break;
        }
        current = &(*current)->next;
    }

    pthread_mutex_unlock(&clientsMutex);
}

void broadcastToClients(const char *message, int senderSocket) {
    pthread_mutex_lock(&clientsMutex);

    ClientInfo *current = clients;
    while (current != NULL) {
        if (current->socket != senderSocket) {
            send(current->socket, message, strlen(message), 0);
        }
        current = current->next;
    }

    pthread_mutex_unlock(&clientsMutex);
}

void stopServer(void) {
    if (!serverRunning) {
        return;
    }

    serverRunning = false;

    if (serverSocket >= 0) {
        close(serverSocket);
        serverSocket = -1;
    }

    pthread_mutex_lock(&clientsMutex);
    ClientInfo *current = clients;
    while (current != NULL) {
        close(current->socket);
        ClientInfo *next = current->next;
        free(current);
        current = next;
    }
    clients = NULL;
    pthread_mutex_unlock(&clientsMutex);

    printf("Server stopped\n");
}

bool isServerRunning(void) {
    return serverRunning;
}