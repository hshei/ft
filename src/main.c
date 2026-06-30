#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <unistd.h>
#include <zlib.h>
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
        // skipping . and .. hidden directories
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;
        
        // building the full path for each file
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
            // file not found
            snprintf(err, sizeof(err), "not found %s", args[i]);
            ft_error(err);
        }

        if (S_ISDIR(st.st_mode)) {
            // if the args is a directory, traverse it
            collect_files(args[i], files);
        } else {
            // push to the vectors
            char *path = strdup(args[i]);
            vector_push(files, &path);
        }

    }
}

int main(int argc, char **argv){
    const char *usage = "Usage: ft send <ip> <file>\n"
                    "       ft send <file>\n"
                    "       ft receive\n";

    ft_options opts = {0};
    opts.compress = false;
    opts.comp_level = Z_DEFAULT_COMPRESSION;
    
    int flag;
    // "c::" the (::) meaning optional number after the flag
    while ((flag = getopt(argc, argv, "c::")) != -1){
        switch (flag){
            case 'c':
                opts.compress = true;
                if (optarg) opts.comp_level = atoi(optarg);
                break;
            default:
                printf("%s", usage);
                return EXIT_FAILURE;
        }
    }

    if (optind >= argc) {printf("%s", usage); return EXIT_FAILURE;}

    // optind tell you where the positional arguments begin
    char *command = argv[optind];

    if (strcmp(command, "send") == 0) {
        vector_s *files = NULL;
        vector_init(&files, sizeof(char *));

        struct stat st;
        const char *ip = NULL;
        char ip_buf[INET_ADDRSTRLEN];

        // files and dir start at optind + 1
        if (stat(argv[optind + 1], &st) == -1 && (argc - optind) > 2) {
            // check if there is at least one file
            if (optind + 1 >= argc) { printf("%s", usage); return EXIT_FAILURE; }
            // not a file and there are more args — treat as IP
            ip = argv[optind + 1];
            // everything after that are files or dirs
            build_file_list(&argv[optind + 2], argc - optind - 2, files);
        } else {
            // no ip is provided: eveything after the command are files or dirs, and auto-discovery called 
            build_file_list(&argv[optind + 1], argc - optind - 1, files);
            discovery_find(ip_buf, sizeof(ip_buf));
            ip = ip_buf;
        }
        
        // all files with their relative path are the files vector, calling sending
        sender_run(ip, files, &opts);
        vector_free(files);
    } else if (strcmp(command, "receive") == 0) {
        receiver_run();
    } else {
        printf("%s", usage);
        return EXIT_FAILURE;
    }


    return EXIT_SUCCESS;
}