#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "helper.h"

#define PORT "5679"
#define DISCOVERY_PORT 5680
#define BACKLOG 10


int discovery_listen(int tcp_fd) {
    // set up UDP discovery
    int udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in udp_addr;
    memset(&udp_addr, 0, sizeof(udp_addr));
    udp_addr.sin_family = AF_INET;
    udp_addr.sin_port = htons(DISCOVERY_PORT);
    udp_addr.sin_addr.s_addr = INADDR_ANY;
    bind(udp_fd, (struct sockaddr *)&udp_addr, sizeof(udp_addr));

    printf("waiting for connection...\n");

    // poll both
    struct pollfd pfds[2];
    pfds[0].fd = tcp_fd;
    pfds[0].events = POLLIN;
    pfds[1].fd = udp_fd;
    pfds[1].events = POLLIN;

    int connected = 0;
    int new_fd = -1;

    while (!connected) {
        poll(pfds, 2, -1);

        if (pfds[1].revents & POLLIN) {
            // UDP discovery request — respond
            char buf[32];
            struct sockaddr_in from_addr;
            socklen_t from_len = sizeof(from_addr);
            recvfrom(udp_fd, buf, sizeof(buf), 0, (struct sockaddr *)&from_addr, &from_len);
            sendto(udp_fd, "FT_HERE", 7, 0, (struct sockaddr *)&from_addr, from_len);

            char sender_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &from_addr.sin_addr, sender_ip, sizeof(sender_ip));
            printf("discovered by %s\n", sender_ip);
        }

        if (pfds[0].revents & POLLIN) {
            // TCP connection ready
            struct sockaddr_storage their_addr;
            socklen_t sin_size = sizeof(their_addr);
            new_fd = accept(tcp_fd, (struct sockaddr *)&their_addr, &sin_size);
            if (new_fd == -1) { perror("accept"); continue; }

            char s[INET_ADDRSTRLEN];
            inet_ntop(their_addr.ss_family,
                get_in_addr((struct sockaddr *)&their_addr), s, sizeof(s));
            printf("connected from %s\n", s);
            connected = 1;
        }
    }

    close(udp_fd);
    return new_fd;
}

int discovery_find(char *ip_out, size_t ip_size) {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    int yes = 1;
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &yes, sizeof(yes));

    // broadcast "FT_DISCOVER"
    struct sockaddr_in bcast_addr;
    memset(&bcast_addr, 0, sizeof(bcast_addr));
    bcast_addr.sin_family = AF_INET;
    bcast_addr.sin_port = htons(DISCOVERY_PORT);
    bcast_addr.sin_addr.s_addr = INADDR_BROADCAST;

    printf("looking for receiver...\n");
    sendto(sock, "FT_DISCOVER", 11, 0, (struct sockaddr *)&bcast_addr, sizeof(bcast_addr));

    // wait for response
    char buf[32];
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&from_addr, &from_len);

    // extract IP
    inet_ntop(AF_INET, &from_addr.sin_addr, ip_out, ip_size);
    printf("found receiver at %s\n", ip_out);

    close(sock);
    return 0;
}
