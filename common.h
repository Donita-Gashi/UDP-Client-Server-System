#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

void init_storage();
void process_file_command(int sockfd, char *buffer, struct sockaddr_in *client_addr);

#endif
