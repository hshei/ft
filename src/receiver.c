#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "discovery.h"
#include "helper.h"

#define PORT "5678"
#define BACKLOG 10

static void get_sockfd(int *sockfd_out){
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int yes=1;
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE; // use my IP

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        ft_error("failed to resolve address");
    }

    // loop through all the results and bind to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) ft_error("failed to configure socket");

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo); // all done with this structure
    
    if (p == NULL)  {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    *sockfd_out = sockfd;
}

static void make_dirs(const char *filepath) {
    char path[1024];
    strncpy(path, filepath, sizeof(path) - 1);
    path[sizeof(path) - 1] = '\0';

    // walk through each '/' and create the directory up to that point
    for (char *p = path + 1; *p; p++) {
        if (*p == '/') {
            *p = '\0';                    // temporarily terminate
            mkdir(path, 0755);            // create this dir level
            *p = '/';                     // restore
        }
    }
}

void receiver_run(void){
    int sockfd; get_sockfd(&sockfd);
    if (listen(sockfd, BACKLOG) == -1) ft_error("failed to listen on port");

    // sending upd broadcast
    int new_fd = discovery_listen(sockfd);
    
    int file_count;
    recv(new_fd, &file_count, sizeof(int), 0);

    for (int i = 0; i < file_count; i++){
        // recv name_len 
        uint16_t name_len;
        recv(new_fd, &name_len, 2, 0);

        // recv filename
        char filename[256];
        recv(new_fd, filename, name_len, 0);
        filename[name_len] = '\0';

        // reject path traversal
        if (strstr(filename, "..") != NULL) {
            ft_error("rejected unsafe path");
        }

        // recv filesize 
        uint64_t filesize;
        recv(new_fd, &filesize, 8, 0);

        // formatting the receiving message 
        char size_str[32];
        format_size(filesize, size_str, sizeof(size_str));
        printf("receiving %s (%s)\n", filename, size_str);
        
        // create parent directories
        make_dirs(filename);

        // opening the file for receive
        FILE *fp = fopen(filename, "wb");
        if (!fp) { close(new_fd); ft_error("can't create file");}

        char chunk[4096];
        uint64_t total = 0;
        time_t start = time(NULL);
        int bar_width = 30;
        char speed_str[32];
        // receiving loop
        while (total < filesize) {
            // if the file is less than the chunk size, read the file size only
            size_t remaining = filesize - total;
            size_t to_read = remaining < sizeof(chunk) ? remaining: sizeof(chunk);
            ssize_t n = recv(new_fd, chunk, to_read, 0);
            if (n <= 0) {
                printf("\ntransfer interrupted\n");
                fclose(fp);
                close(new_fd);
                return;
            }
            fwrite(chunk, 1, n, fp);
            total += n;

            // progress bar with the pretty formatting
            int percent = (int)((total * 100) / filesize);
            int filled = (percent * bar_width) / 100;

            time_t elapsed = time(NULL) - start;
            double speed = elapsed > 0 ? (double)total / elapsed : 0;
            int eta = speed > 0 ? (int)((filesize - total) / speed) : 0;

            format_size(total, size_str, sizeof(size_str));

            format_size((uint64_t)speed, speed_str, sizeof(speed_str));

            printf("\r[");
            for (int j = 0; j < bar_width; j++)
                printf(j < filled ? "#" : " ");
            printf("] %d%%  %s  %s/s  ETA %ds", percent, size_str, speed_str, eta);
            fflush(stdout);
        }   
        printf("\n");

        printf("\nsaved %s (%llu bytes)\n", filename, filesize);
        fclose(fp);
    }

    close(new_fd);
}