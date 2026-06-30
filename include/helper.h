#ifndef HELPER_H
#define HELPER_H

#include <stdbool.h>
#define CHUNK_SIZE 32768 // 32KB

typedef struct {
    bool compress;
    int comp_level;
    bool encrypt;
    unsigned char key[32];
} ft_options;

void *get_in_addr(struct sockaddr *sa);
void format_size(uint64_t bytes, char *out, size_t out_size);
void ft_error(const char *msg);

#endif // HELPER_H