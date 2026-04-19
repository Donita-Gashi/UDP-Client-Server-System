#include "common.h"
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>

#define STORAGE_DIR "storage"

static void send_text_udp(int sockfd, const char *text, struct sockaddr_in *client_addr) {
    sendto(sockfd, text, strlen(text), 0, (struct sockaddr *)client_addr, sizeof(*client_addr));
}

static int is_valid_filename(const char *filename) {
    if (filename == NULL || strlen(filename) == 0) return 0;
    if (strstr(filename, "..") != NULL) return 0;
    if (strchr(filename, '/') != NULL) return 0;
    if (strchr(filename, '\\') != NULL) return 0;
    return 1;
}

static void build_path(char *path, size_t size, const char *filename) {
    snprintf(path, size, "%s/%s", STORAGE_DIR, filename);
}

void init_storage() {
    struct stat st = {0};
    if (stat(STORAGE_DIR, &st) == -1) {
        mkdir(STORAGE_DIR, 0777);
    }
}

void cmd_list(int sockfd, struct sockaddr_in *client_addr) {
    DIR *dir = opendir(STORAGE_DIR);
    if (!dir) {
        send_text_udp(sockfd, "Gabim: storage nuk mund te hapet.\n", client_addr);
        return;
    }

    struct dirent *entry;
    char response[4096];
    response[0] = '\0';

    strcat(response, "LISTA E FILE-VE:\n");

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            strcat(response, "- ");
            strcat(response, entry->d_name);
            strcat(response, "\n");
        }
    }

    closedir(dir);

    if (strcmp(response, "LISTA E FILE-VE:\n") == 0) {
        strcat(response, "(Nuk ka file)\n");
    }

    send_text_udp(sockfd, response, client_addr);
}

void cmd_read(int sockfd, struct sockaddr_in *client_addr, const char *filename) {
    if (!is_valid_filename(filename)) {
        send_text_udp(sockfd, "Gabim: emer file-i i pavlefshem.\n", client_addr);
        return;
    }

    char path[512];
    build_path(path, sizeof(path), filename);

    FILE *fp = fopen(path, "r");
    if (!fp) {
        send_text_udp(sockfd, "Gabim: file nuk u gjet.\n", client_addr);
        return;
    }

    char response[4096];
    char line[512];

    response[0] = '\0';
    strcat(response, "PERMBAJTJA E FILE-IT:\n");

    while (fgets(line, sizeof(line), fp)) {
        strcat(response, line);
    }

    fclose(fp);
    send_text_udp(sockfd, response, client_addr);
}

void cmd_delete(int sockfd, struct sockaddr_in *client_addr, const char *filename) {
    if (!is_valid_filename(filename)) {
        send_text_udp(sockfd, "Gabim: emer file-i i pavlefshem.\n", client_addr);
        return;
    }

    char path[512];
    build_path(path, sizeof(path), filename);

    if (remove(path) == 0) {
        send_text_udp(sockfd, "File u fshi me sukses.\n", client_addr);
    } else {
        send_text_udp(sockfd, "Gabim: file nuk u fshi.\n", client_addr);
    }
}

void cmd_search(int sockfd, struct sockaddr_in *client_addr, const char *keyword) {
    if (!keyword || strlen(keyword) == 0) {
        send_text_udp(sockfd, "Gabim: jep nje fjale per kerkim.\n", client_addr);
        return;
    }

    DIR *dir = opendir(STORAGE_DIR);
    if (!dir) {
        send_text_udp(sockfd, "Gabim: storage nuk mund te hapet.\n", client_addr);
        return;
    }

    struct dirent *entry;
    char response[4096];
    response[0] = '\0';

    strcat(response, "REZULTATET E KERKIMIT:\n");

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG && strstr(entry->d_name, keyword)) {
            strcat(response, "- ");
            strcat(response, entry->d_name);
            strcat(response, "\n");
        }
    }

    closedir(dir);

    if (strcmp(response, "REZULTATET E KERKIMIT:\n") == 0) {
        strcat(response, "Asnje file nuk u gjet.\n");
    }

    send_text_udp(sockfd, response, client_addr);
}

void cmd_info(int sockfd, struct sockaddr_in *client_addr, const char *filename) {
    if (!is_valid_filename(filename)) {
        send_text_udp(sockfd, "Gabim: emer file-i i pavlefshem.\n", client_addr);
        return;
    }

    char path[512];
    build_path(path, sizeof(path), filename);

    struct stat st;
    if (stat(path, &st) != 0) {
        send_text_udp(sockfd, "Gabim: file nuk u gjet.\n", client_addr);
        return;
    }

    char response[1024];
    snprintf(response, sizeof(response),
             "INFO PER FILE:\n"
             "Emri: %s\n"
             "Madhesia: %ld bytes\n",
             filename, (long)st.st_size);

    send_text_udp(sockfd, response, client_addr);
}

void cmd_upload(int sockfd, struct sockaddr_in *client_addr, const char *filename) {
   /*
   Kjo pjese eshte lene per integrim final me:
   - ANËTARIN 1 (server.c)-Zgjimi
   - ANËTARIN 3 (client.c)-Zana

   Upload ne UDP duhet te perputhet me menyren si klienti dergon file
   dhe si serveri i pranon paketat (sendto/recvfrom).

   Çka duhet en fund:
   - Percakto formatin e dergimit te file-it nga klienti
   - Perdor recvfrom() per te marre pjesët e file-it
   - Ruaj file-in në folderin "storage"
*/
    send_text_udp(sockfd, "Upload do te implementohet ne integrimin final.\n", client_addr);
}

void cmd_download(int sockfd, struct sockaddr_in *client_addr, const char *filename) {
    /*
       Për momentin download përdor logjikën e /read.
       Në fund mund të përshtatet sipas mënyrës që grupi e vendos për transferimin e file-it. 
       Kjo pjese eshte lene qe te bashpunohet me antarin 3-Zana,
       
       cka duhet:
        Përdor sendto() për të dërguar file-in
        Dërgo sinjal fillimi dhe fundi (p.sh. START/END)
    */
    cmd_read(sockfd, client_addr, filename);
}

void process_file_command(int sockfd, char *buffer, struct sockaddr_in *client_addr) {
    char command[256];
    char arg[256];

    memset(command, 0, sizeof(command));
    memset(arg, 0, sizeof(arg));

    sscanf(buffer, "%s %255[^\n]", command, arg);

    if (strcmp(command, "/list") == 0) {
        cmd_list(sockfd, client_addr);
    }
    else if (strcmp(command, "/read") == 0) {
        if (strlen(arg) == 0) {
            send_text_udp(sockfd, "Perdorimi: /read <filename>\n", client_addr);
            return;
        }
        cmd_read(sockfd, client_addr, arg);
    }
    else if (strcmp(command, "/upload") == 0) {
        if (strlen(arg) == 0) {
            send_text_udp(sockfd, "Perdorimi: /upload <filename>\n", client_addr);
            return;
        }
        cmd_upload(sockfd, client_addr, arg);
    }
    else if (strcmp(command, "/download") == 0) {
        if (strlen(arg) == 0) {
            send_text_udp(sockfd, "Perdorimi: /download <filename>\n", client_addr);
            return;
        }
        cmd_download(sockfd, client_addr, arg);
    }
    else if (strcmp(command, "/delete") == 0) {
        if (strlen(arg) == 0) {
            send_text_udp(sockfd, "Perdorimi: /delete <filename>\n", client_addr);
            return;
        }
        cmd_delete(sockfd, client_addr, arg);
    }
    else if (strcmp(command, "/search") == 0) {
        if (strlen(arg) == 0) {
            send_text_udp(sockfd, "Perdorimi: /search <keyword>\n", client_addr);
            return;
        }
        cmd_search(sockfd, client_addr, arg);
    }
    else if (strcmp(command, "/info") == 0) {
        if (strlen(arg) == 0) {
            send_text_udp(sockfd, "Perdorimi: /info <filename>\n", client_addr);
            return;
        }
        cmd_info(sockfd, client_addr, arg);
    }
    else {
        /*
           Nëse grupi ka komanda tjera në server.c,
           ato mund të trajtohen atje (Antari 1-Zgjimi), ose të shtohen këtu më vonë nga (Antari 2-Zymeri).
            ky funksion mund te zgjerohet sipas nevojes se projektit.
           */
          
        send_text_udp(sockfd, "Komande e panjohur.\n", client_addr);
    }
}

