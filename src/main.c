#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/stat.h>


#include "sender.h"
#include "receiver.h"
#include "discovery.h"

int main(int argc, char **argv){
    const char *usage = "Usage: ft send <ip> <file>\n"
                        "       ft send <file>\n"
                        "       ft receive\n";

    if (argc < 2) {
        printf("%s", usage);
        return EXIT_FAILURE;
    }
    if (strcmp(argv[1], "send") == 0) {
        if (argc < 3) { printf("%s", usage); return EXIT_FAILURE; }

        struct stat st;
        if (argc == 3) {
            if (stat(argv[2], &st) == -1) {
                fprintf(stderr, "ft: file not found: %s\n", argv[2]);
                return EXIT_FAILURE;
            }
            char ip[INET_ADDRSTRLEN];
            discovery_find(ip, sizeof(ip));
            sender_run(ip, &argv[2], 1);
        } else {
            if (stat(argv[2], &st) == -1) {
                sender_run(argv[2], &argv[3], argc - 3);
            } else {
                char ip[INET_ADDRSTRLEN];
                discovery_find(ip, sizeof(ip));
                sender_run(ip, &argv[2], argc - 2);
            }
        }
    } else if (strcmp(argv[1], "receive") == 0) {
        receiver_run();
    } else {
        printf("%s", usage);
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}