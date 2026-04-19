#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "common.h"

#define PORT 8080

void *http_server(void *arg) {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[3000];

    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 5);

    printf("HTTP server running on port %d...\n", PORT);

    while (1) {
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

        memset(buffer, 0, sizeof(buffer));
        read(new_socket, buffer, 3000);

        if (strstr(buffer, "GET /stats") != NULL) {

            char response[5000];
            char clients_info[1000] = "";
            char logs_info[2000] = "";

            // klientët
            for (int i = 0; i < client_count; i++) {
                char temp[100];
                sprintf(temp, "Client %d: %s\n", i+1, clients[i].ip);
                strcat(clients_info, temp);
            }

            // log-et
            for (int i = 0; i < total_messages && i < MAX_LOGS; i++) {
                strcat(logs_info, logs[i]);
                strcat(logs_info, "\n");
            }

            sprintf(response,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n\r\n"
                "Active clients: %d\n"
                "Total messages: %d\n\n"
                "Clients:\n%s\n"
                "Logs:\n%s\n",
                client_count, total_messages, clients_info, logs_info
            );

            send(new_socket, response, strlen(response), 0);
        }

        close(new_socket);
    }
}
