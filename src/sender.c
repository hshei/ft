#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <sys/socket.h>
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
    hints.ai_family = AF_INET;
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
            perror("client: connect");
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

void sender_run(const char *ip, vector_s *files){
    int file_count = files->size;
    // getting the socket fd
    int sockfd; get_sockfd(ip, &sockfd);
    // sending the file count
    send(sockfd, &file_count, sizeof(int), 0);
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
        send(sockfd, &name_len, 2, 0);
        send(sockfd, file, name_len, 0);
        send(sockfd, &filesize, 8, 0);
                
        // sending file data in chuncks
        FILE *fp = fopen(file, "rb");
        if (fp == NULL){
            snprintf(err, sizeof(err), "can't open file: %s", file);
            ft_error(err);
        }
    
        char chunk[4096] = {0};
        size_t bytes_read = 0;
        while ((bytes_read = fread(chunk, 1, sizeof(chunk), fp)) > 0){
            if (send(sockfd, chunk, bytes_read, 0) == -1) {
                perror("send failed");
                fclose(fp);
                close(sockfd);
                return;
            }        
        }

        format_size(filesize, size_str, sizeof(size_str));
        printf("sent %s (%s)\n", file, size_str);     

        fclose(fp);
    }

    printf("\ndone — %d file(s) sent\n", file_count);
    close(sockfd);
}