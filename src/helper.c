#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void format_size(uint64_t bytes, char *out, size_t out_size) {
    if (bytes >= 1024 * 1024 * 1024)
        snprintf(out, out_size, "%.1f GB", (double)bytes / (1024 * 1024 * 1024));
    else if (bytes >= 1024 * 1024)
        snprintf(out, out_size, "%.1f MB", (double)bytes / (1024 * 1024));
    else if (bytes >= 1024)
        snprintf(out, out_size, "%.1f KB", (double)bytes / 1024);
    else
        snprintf(out, out_size, "%llu B", bytes);
}

void ft_error(const char *msg) {
    fprintf(stderr, "ft: %s\n", msg);
    exit(EXIT_FAILURE);
}
