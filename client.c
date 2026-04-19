#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER_IP "127.0.0.1"
#define PORT 5000
#define BUFFER_SIZE 1024

// Funksioni per kontrollin e privilegjeve
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
    int sockfd;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char role[20];

    // Krijimi i socket-it ne Linux
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Gabim ne socket");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    printf("Zgjedh rolin (admin / read): ");
    scanf("%19s", role);
    getchar(); // Pastron newline nga buffer-i

    printf("U lidhe si: %s\n", role);

    while (1) {
        printf("\nShkruaj komanden: ");
        if (fgets(buffer, BUFFER_SIZE, stdin) == NULL) break;

        buffer[strcspn(buffer, "\n")] = 0; // Heq \n ne fund

        if (strcmp(buffer, "exit") == 0) {
            printf("Duke u mbyllur...\n");
            break;
        }

        if (!is_allowed(role, buffer)) {
            printf("Nuk ke privilegje per kete komande!\n");
            continue;
        }

        // Dergimi i mesazhit
        sendto(sockfd, buffer, strlen(buffer), 0,
               (struct sockaddr*)&server_addr, sizeof(server_addr));

        // Marrja e pergjigjes
        socklen_t len = sizeof(server_addr);
        int n = recvfrom(sockfd, buffer, BUFFER_SIZE - 1, 0,
                         (struct sockaddr*)&server_addr, &len);

        if (n < 0) {
            perror("Gabim ne marrje");
            continue;
        }

        buffer[n] = '\0';
        printf("Serveri: %s\n", buffer);
    }

    close(sockfd);
    return 0;
}
