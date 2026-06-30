#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <zlib.h>
#include <arpa/inet.h>

#include "helper.h"
#include "datastructures.h"

#define PORT "5678"

static void get_sockfd(const char *ip, int *sockfd_out){
    int sockfd;  
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET; // ipv4
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(ip, PORT, &hints, &servinfo)) != 0) {
        ft_error("failed to resolve address");
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) continue;

        inet_ntop(p->ai_family,
            get_in_addr((struct sockaddr *)p->ai_addr),
            s, sizeof s);

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }

        break;
    }

    if (p == NULL) {
        ft_error("failed to connect to receiver");
    }

    freeaddrinfo(servinfo); 

    *sockfd_out = sockfd;
}

int send_all(int fd, const char *buf, size_t len) {    
    size_t total = 0;
    while (total < len) {
        ssize_t n = send(fd, buf + total, len - total, 0);
        if (n <= 0) return -1;
        total += n;
    }
    return 0;
}

void sender_run(const char *ip, vector_s *files, ft_options *opts){
    int file_count = files->size;
    // getting the socket fd
    int sockfd; get_sockfd(ip, &sockfd);
    // sending the file count
    send_all(sockfd, (char *)&file_count, sizeof(int));
    // sending the compress flag once for the session
    uint8_t compress_flag = opts->compress ? 1 : 0;
    send_all(sockfd, (char *)&compress_flag, 1);
    printf("sending %d file(s)\n\n", file_count);

    char size_str[32];
    char err[256];
    struct stat st;

    for (int i = 0; i < file_count; i++){
        char *file; vector_get(files, i, &file);
        // getting the file size
        stat(file, &st);
        uint64_t filesize = st.st_size;

        // getting the name length
        uint16_t name_len = strlen(file);

        // sending header: name_len + filename + filesize
        send_all(sockfd, (char *)&name_len, 2);
        send_all(sockfd, file, name_len);
        send_all(sockfd, (char *)&filesize, 8);
                
        // sending file data in chuncks
        FILE *fp = fopen(file, "rb");
        if (fp == NULL){
            snprintf(err, sizeof(err), "can't open file: %s", file);
            ft_error(err);
        }
    
        char chunk[CHUNK_SIZE];
        char compressed[CHUNK_SIZE * 2]; //compression can be a little bigger in the worst case
        size_t bytes_read = 0;

        uint64_t total_sent = 0;
        int chunk_num = 0;
        while ((bytes_read = fread(chunk, 1, sizeof(chunk), fp)) > 0){
            if ((opts->compress)){
                uLongf comp_len = sizeof(compressed);
                compress2((Bytef *)compressed, &comp_len, (Bytef *)chunk, bytes_read, opts->comp_level);

                // sending the compressed chunk size before the chunk
                uint32_t clen = (uint32_t)comp_len;
                send_all(sockfd, (char *)&clen, 4);
                send_all(sockfd, compressed, comp_len);
            } else {
                 uint32_t clen = (uint32_t)bytes_read;
                if (send_all(sockfd, (char *)&clen, 4) < 0) { printf("DEBUG: clen send failed at chunk %d\n", chunk_num); break; }
                if (send_all(sockfd, chunk, bytes_read) < 0) { printf("DEBUG: data send failed at chunk %d\n", chunk_num); break; }
                total_sent += bytes_read;
                chunk_num++;
            }
        }
        
        printf("DEBUG: sender finished, %d chunks, %llu bytes\n", chunk_num, total_sent);

        format_size(filesize, size_str, sizeof(size_str));
        printf("sent %s (%s)\n", file, size_str);     
        
        fclose(fp);        
    }
    
    printf("\ndone — %d file(s) sent\n", file_count);
    close(sockfd);
}