#include <string.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 5000
#define BUFFER_SIZE 1024

int is_allowed(const char *role, const char *command) {
    if (!role || !command) {
        return 0;
    }
    if (strcmp(role, "admin") == 0) {
        return 1;
    }
    if (strcmp(role, "read") == 0) {
        if (strncmp(command, "/list", 5) == 0) return 1;
        if (strncmp(command, "/read", 5) == 0) return 1;
        if (strncmp(command, "/search", 7) == 0) return 1;
        if (strncmp(command, "/info", 5) == 0) return 1;
        return 0;
    }
    return 0;
}

int main() {
    WSADATA wsa;
    SOCKET sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char role[20];

    if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) {
        printf("Gabim në Winsock\n");
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("Gabim në socket\n");
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    printf("Zgjedh rolin (admin / read): ");
    scanf("%19s", role);
    getchar();

    printf("U lidhe si: %s\n", role);

    while (1) {
        printf("\nShkruaj komanden: ");
        fgets(buffer, BUFFER_SIZE, stdin);

        buffer[strcspn(buffer, "\n")] = 0;

        if (strcmp(buffer, "exit") == 0) {
            printf("Duke u mbyllur...\n");
            break;
        }

        if (!is_allowed(role, buffer)) {
            printf("Nuk ke privilegje per kete komande!\n");
            continue;
        }

        sendto(sockfd, buffer, strlen(buffer), 0,
               (struct sockaddr*)&server_addr, sizeof(server_addr));

        int len = sizeof(server_addr);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr*)&server_addr, &len);

        if (n == SOCKET_ERROR) {
            printf("Gabim në marrje\n");
            continue;
        }

        buffer[n] = '\0';
        printf("Serveri: %s\n", buffer);
    }

    closesocket(sockfd);
    WSACleanup();

    return 0;
}
