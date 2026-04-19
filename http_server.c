#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "common.h"

#define PORT 8080

void *handle_request(void *arg) {
    int new_socket = *(int*)arg;
    free(arg);

    char buffer[3000];
    memset(buffer, 0, sizeof(buffer));
    read(new_socket, buffer, sizeof(buffer));

    if (strstr(buffer, "GET /stats") != NULL) {

        char clients_info[1000] = "";
        char logs_info[2000] = "";
        char body[4000];
        char response[5000];

        // klientet
        for (int i = 0; i < client_count; i++) {
            char temp[100];
            sprintf(temp, "Client %d: %s\n", i + 1, clients[i].ip);
            strcat(clients_info, temp);
        }

        // log-et
        for (int i = 0; i < total_messages && i < MAX_LOGS; i++) {
            strcat(logs_info, logs[i]);
            strcat(logs_info, "\n");
        }

        int is_json = strstr(buffer, "format=json") != NULL;

        if (is_json) {
            sprintf(body,
                "{\n"
                "  \"active_clients\": %d,\n"
                "  \"total_messages\": %d\n"
                "}\n",
                client_count, total_messages
            );

            sprintf(response,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/json\r\n"
                "Content-Length: %ld\r\n\r\n"
                "%s",
                strlen(body), body
            );

        } else {
            sprintf(body,
                "Active clients: %d\n"
                "Total messages: %d\n\n"
                "Clients:\n%s\n"
                "Logs:\n%s\n",
                client_count, total_messages, clients_info, logs_info
            );

            sprintf(response,
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: %ld\r\n\r\n"
                "%s",
                strlen(body), body
            );
        }

        send(new_socket, response, strlen(response), 0);
    } else {
        char *not_found =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/plain\r\n\r\n"
            "Not Found";

        send(new_socket, not_found, strlen(not_found), 0);
    }

    close(new_socket);
    return NULL;
}


void *http_server(void *arg) {
    int server_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("HTTP server running on port %d...\n", PORT);

    while (1) {
        int *new_socket = malloc(sizeof(int));
        *new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

        if (*new_socket < 0) {
            perror("accept failed");
            free(new_socket);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, handle_request, new_socket);
        pthread_detach(tid);
    }

    return NULL;
}
