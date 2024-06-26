#include <memory>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "spdlog/spdlog.h"

#include "payload.h"

int main() {
    spdlog::set_pattern("[%m%d %H:%M:%S.%e] [%^%l%$] %v");
    int sockfd;
    struct sockaddr_in server_addr;
    udp_data_t packet;
    int seq = 0;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080); 
    server_addr.sin_addr.s_addr = inet_addr("192.168.3.66"); 

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 1000000L; 
    
    while (1) {
        packet.checksum = seq++;
        if (sendto(sockfd, &packet, sizeof(packet), 0, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            perror("sendto failed");
            exit(EXIT_FAILURE);
        }
        nanosleep(&ts, NULL); 
    }
    close(sockfd);
    return 0;
}
