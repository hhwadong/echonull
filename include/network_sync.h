#ifndef NETWORK_SYNC_H
#define NETWORK_SYNC_H

#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "contact_manager.h"

typedef struct ClientInfo {
    int socket;
    struct sockaddr_in address;
    struct ClientInfo *next;
} ClientInfo;

void startServer(int port);
bool syncContacts(const char *serverIP, int port, ContactNode **localContacts);
void stopServer(void);
bool isServerRunning(void);

#endif