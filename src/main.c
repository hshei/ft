#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <sys/stat.h>


#include "sender.h"
#include "receiver.h"
#include "discovery.h"
#include "helper.h"
#include "datastructures.h"

void collect_files(const char *dir_path, vector_s *files){
    char full_path[1024];

    DIR *dir = opendir(dir_path);
    if (!dir) return;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL){
        // skipping . and ..
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        // building the full path
        snprintf(full_path, sizeof(full_path), "%s/%s", dir_path, entry->d_name);

        struct stat st;
        stat(full_path, &st);

        if (S_ISDIR(st.st_mode)) collect_files(full_path, files);
        else {
            char *path = strdup(full_path);
            vector_push(files, &path);
        }
    }
    closedir(dir);
}


void build_file_list(char **args, int arg_count, vector_s *files){
    struct stat st;
    char err[256];
    for (int i = 0; i < arg_count; i++){
        if (stat(args[i], &st) == -1){
            snprintf(err, sizeof(err), "not found %s", args[i]);
            ft_error(err);
        }

        if (S_ISDIR(st.st_mode)) {
            collect_files(args[i], files);
        } else {
            char *path = strdup(args[i]);
            vector_push(files, &path);
        }

    }
}

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

        vector_s *files = NULL;
        vector_init(&files, sizeof(char *));

        struct stat st;
        const char *ip = NULL;
        char ip_buf[INET_ADDRSTRLEN];

        // first arg after "send": IP or file?
        if (stat(argv[2], &st) == -1 && argc > 3) {
            // not a file and there are more args — treat as IP
            ip = argv[2];
            build_file_list(&argv[3], argc - 3, files);
        } else {
            // auto-discover
            build_file_list(&argv[2], argc - 2, files);
            discovery_find(ip_buf, sizeof(ip_buf));
            ip = ip_buf;
        }

        sender_run(ip, files);
        vector_free(files);
    } else if (strcmp(argv[1], "receive") == 0) {
        receiver_run();
    } else {
        printf("%s", usage);
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}