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
#include <openssl/rand.h>

#include "helper.h"
#include "crypto.h"
#include "datastructures.h"

#define PORT "5678"

static void get_sockfd(const char *ip, int *sockfd_out){
    // this code is almost baseline, you can get it from beejs network tutorial
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
    // send() does not guarantee sending all the bytes in one-go, so this is the solution
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
    // sending the compress flag for the session
    uint8_t compress_flag = opts->compress ? 1 : 0;
    send_all(sockfd, (char *)&compress_flag, 1);
    // sending the encrpty flag for the session
    uint8_t encrypt_flag = opts->encrypt ? 1 : 0;
    send_all(sockfd, (char *)&encrypt_flag, 1);
    
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

        while ((bytes_read = fread(chunk, 1, sizeof(chunk), fp)) > 0){
            unsigned char *payload = (unsigned char *)chunk;
            int payload_len = bytes_read;

            if ((opts->compress)){
                uLongf comp_len = sizeof(compressed);
                compress2((Bytef *)compressed, &comp_len, (Bytef *)chunk, bytes_read, opts->comp_level);

                payload = (unsigned char *)compressed;
                payload_len = comp_len;
            }

            //  encrypt after compress (if both), or just encrypt 
            if (opts->encrypt){
                unsigned char iv[16];
                // random 16-bytes value that maes encrption non-deterministic
                RAND_bytes(iv, 16);   

                unsigned char enc[CHUNK_SIZE * 2 + 16];
                int enc_len = encrypt_chunk(opts->key, iv, payload, payload_len, enc);

                send_all(sockfd, (char *)iv, 16);              // IV first
                payload = enc;
                payload_len = enc_len;
            } 

            uint32_t clen = (uint32_t)payload_len;
            send_all(sockfd, (char *)&clen, 4);
            send_all(sockfd, (char *)payload, payload_len);
        }
        
        format_size(filesize, size_str, sizeof(size_str));
        printf("sent %s (%s)\n", file, size_str);     
        
        fclose(fp);        
    }
    
    printf("\ndone — %d file(s) sent\n", file_count);
    close(sockfd);
}