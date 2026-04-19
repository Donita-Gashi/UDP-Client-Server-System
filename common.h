#ifndef COMMON_H
#define COMMON_H

#include <netinet/in.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define UDP_PORT 5000
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define TIMEOUT_SEC 60

typedef struct {
    struct sockaddr_in addr;
    time_t last_active;
    int is_active;
} UDPClient;

extern int total_messages_received;
extern UDPClient clients[MAX_CLIENTS];

void init_storage();
void process_file_command(int sockfd, char *buffer, struct sockaddr_in *client_addr);
void *http_server(void *arg);

#endif
