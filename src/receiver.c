#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <zlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "discovery.h"
#include "crypto.h"
#include "helper.h"

#define PORT "5678"
#define BACKLOG 10

static int recv_all(int fd, char *buf, size_t len) {
    // recv does not guarantee all the sent data on one-go, so this is the solution
    size_t total = 0;
    while (total < len) {
        ssize_t n = recv(fd, buf + total, len - total, 0);
        if (n <= 0) return -1;
        total += n;
    }
    return 0;
}

static void get_sockfd(int *sockfd_out){
    // this code is almost baseline, you can see it on beejs network tutorial
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
                p->ai_protocol)) == -1)  continue;

        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
                sizeof(int)) == -1) ft_error("failed to configure socket");

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        break;
    }

    freeaddrinfo(servinfo); // all done with this structure
    
    if (p == NULL) ft_error("failed to bind to port");

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

void receiver_run(ft_options *opts){
    int sockfd; get_sockfd(&sockfd);
    if (listen(sockfd, BACKLOG) == -1) ft_error("failed to listen on port");

    // getting the tcp connection when ip is used, or connection from udp discovery if a local device connects
    int new_fd = discovery_listen(sockfd);
    
    // recv file count
    int file_count;
    recv_all(new_fd, (char *)&file_count, sizeof(int));

    // recv the compress flag
    uint8_t compress_flag = 0;
    recv_all(new_fd, (char *)&compress_flag, 1);

    // recv the encrypt flag
    uint8_t encrypt_flag = 0;
    recv_all(new_fd, (char *)&encrypt_flag, 1);

    for (int i = 0; i < file_count; i++){
        // recv name_len 
        uint16_t name_len;
        recv_all(new_fd, (char *)&name_len, 2);

        // recv filename
        char filename[1024];
        recv_all(new_fd, filename, name_len);
        filename[name_len] = '\0';

        // reject path traversal
        if (strstr(filename, "..") != NULL) {
            ft_error("rejected unsafe path");
        }

        // recv filesize 
        uint64_t filesize;
        recv_all(new_fd, (char *)&filesize, 8);

        // formatting the receiving message 
        char size_str[32];
        format_size(filesize, size_str, sizeof(size_str));
        printf("receiving %s (%s)\n", filename, size_str);
        
        // create parent directories
        make_dirs(filename);

        // opening the file for receive
        FILE *fp = fopen(filename, "wb");
        if (!fp) { close(new_fd); ft_error("can't create file");}

        char chunk[CHUNK_SIZE];
        char compressed[CHUNK_SIZE * 2]; //compression can be a little bigger than original in worst case
        unsigned char enc[CHUNK_SIZE * 2 + 16]; // AES encrypts in 16-byte blocks, so needs some room when the data is not a mutiple of 16

        uint64_t total = 0;
        time_t start = time(NULL);
        int bar_width = 30;
        char speed_str[32];

        // receiving loop
        while (total < filesize) {
            unsigned char iv[16];

            // read the chunck size
            // stage 1: read IV (only if encrypting)
            if (encrypt_flag) {
                if (recv_all(new_fd, (char *)iv, 16) < 0) {
                    fclose(fp); close(new_fd);
                    ft_error("transfer interrupted");
                }
            }

            // read the length-prefixed payload
            uint32_t clen;
            if (recv_all(new_fd, (char *)&clen, 4) < 0) {
                fclose(fp); close(new_fd);
                ft_error("transfer interrupted");
            }

            if (recv_all(new_fd, (char *)enc, clen) < 0) {
                fclose(fp); close(new_fd);
                ft_error("transfer interrupted");
            }

            unsigned char *payload = enc;
            int payload_len = clen;

            // stage 2: decrypt (reverse of sender's last stage)
            if (encrypt_flag) {
                int dec_len = decrypt_chunk(opts->key, iv, enc, clen, (unsigned char *)compressed);
                if (dec_len < 0) {
                    fclose(fp); close(new_fd);
                    ft_error("decryption failed — wrong password?");
                }
                payload = (unsigned char *)compressed;
                payload_len = dec_len;
            }

            // stage 3: decompress (reverse of sender's first stage)
            size_t write_len;
            if (compress_flag) {
                uLongf dlen = sizeof(chunk);
                uncompress((Bytef *)chunk, &dlen, payload, payload_len);
                fwrite(chunk, 1, dlen, fp);
                write_len = dlen;
            } else {
                fwrite(payload, 1, payload_len, fp);
                write_len = payload_len;
            }

            total += write_len;

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
        fclose(fp);
    }
    close(new_fd);
}