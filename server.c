#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>

int total_messages_received = 0; 

UDPClient clients[MAX_CLIENTS];

int find_or_add_client(struct sockaddr_in *client_addr) {
    time_t now = time(NULL);
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].is_active && 
            clients[i].addr.sin_port == client_addr->sin_port &&
            clients[i].addr.sin_addr.s_addr == client_addr->sin_addr.s_addr) {
            
            clients[i].last_active = now;
            return i;
        }
    }
    
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!clients[i].is_active) {
            clients[i].addr = *client_addr;
            clients[i].is_active = 1;
            clients[i].last_active = now;
            printf("[+] Klient i ri u regjistrua (IP: %s, Port: %d)\n", 
                   inet_ntoa(client_addr->sin_addr), ntohs(client_addr->sin_port));
            return i;
        }
    }
    
    return -1;
}

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    char buffer[BUFFER_SIZE];
    socklen_t addr_len = sizeof(client_addr);
    fd_set readfds;

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].is_active = 0;
    }

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Krijimi i socket deshtoi");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(UDP_PORT);

    if (bind(sockfd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind deshtoi");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("[INFO] Serveri UDP u ndez ne portin %d. Po presim mesazhe...\n", UDP_PORT);

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        struct timeval tv;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        int activity = select(sockfd + 1, &readfds, NULL, NULL, &tv);

        time_t now = time(NULL);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].is_active) {
                if (difftime(now, clients[i].last_active) > TIMEOUT_SEC) {
                    printf("[-] Klienti ne portin %d skadoi (Timeout > %d sekonda inaktivitet).\n", 
                           ntohs(clients[i].addr.sin_port), TIMEOUT_SEC);
                    clients[i].is_active = 0; // Fshijme klientin (Mbyllet "lidhja")
                }
            }
        }

        if (activity < 0) {
            perror("Gabim ne select");
            continue;
        }

        if (FD_ISSET(sockfd, &readfds)) {
            memset(buffer, 0, BUFFER_SIZE);
            
            int n = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
            
            if (n > 0) {
                buffer[n] = '\0'; 
                buffer[strcspn(buffer, "\n")] = 0;

                total_messages_received++;

                int client_idx = find_or_add_client(&client_addr);
                
                if (client_idx != -1) {
                    printf("[Mesazhi %d] Nga [%s:%d]: %s\n", 
                           total_messages_received, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buffer);
                           
                    // 
                    // vendosja e funksioneve
                    // 
                    
                    char reply[BUFFER_SIZE];
                    sprintf(reply, "Serveri UDP: Mesazhi yt '%s' u procesua.\n", buffer);
                    sendto(sockfd, reply, strlen(reply), 0, (const struct sockaddr *)&client_addr, addr_len);
                } else {
                    char *err = "Serveri eshte plot. Lidhja refuzohet.\n";
                    sendto(sockfd, err, strlen(err), 0, (const struct sockaddr *)&client_addr, addr_len);
                }
            }
        }
    }

    close(sockfd);
    return 0;
}
