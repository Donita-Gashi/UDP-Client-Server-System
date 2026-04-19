#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "common.h"

#define HTTP_PORT 8080

void *handle_request(void *arg) {
    int new_socket = *(int*)arg;
    free(arg);

    char buffer[3000];
    memset(buffer, 0, sizeof(buffer));
    read(new_socket, buffer, sizeof(buffer));

    if (strstr(buffer, "GET /stats") != NULL) {
        char clients_info[1000] = "";
        char body[4000];
        char response[5000];
        int active_clients_count = 0;

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].is_active) {
                active_clients_count++;
                char temp[100];
                sprintf(temp, "Client %d: %s:%d\n", 
                        i + 1, 
                        inet_ntoa(clients[i].addr.sin_addr), 
                        ntohs(clients[i].addr.sin_port));
                strcat(clients_info, temp);
            }
        }

        int is_json = strstr(buffer, "format=json") != NULL;

        if (is_json) {
            sprintf(body,
                "{\n"
                "  \"active_clients\": %d,\n"
                "  \"total_messages\": %d\n"
                "}\n",
                active_clients_count, total_messages_received
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
                "Clients:\n%s\n",
                active_clients_count, total_messages_received, clients_info
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
        perror("HTTP socket failed");
        pthread_exit(NULL);
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(HTTP_PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("HTTP bind failed");
        pthread_exit(NULL);
    }

    if (listen(server_fd, 10) < 0) {
        perror("HTTP listen failed");
        pthread_exit(NULL);
    }

    printf("[INFO] HTTP server running on port %d... (Hap ne browser: http://localhost:8080/stats)\n", HTTP_PORT);

    while (1) {
        int *new_socket = malloc(sizeof(int));
        *new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);

        if (*new_socket < 0) {
            perror("HTTP accept failed");
            free(new_socket);
            continue;
        }

        pthread_t tid;
        pthread_create(&tid, NULL, handle_request, new_socket);
        pthread_detach(tid);
    }

    return NULL;
}
