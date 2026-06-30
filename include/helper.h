#ifndef HELPER_H
#define HELPER_H

void *get_in_addr(struct sockaddr *sa);
void format_size(uint64_t bytes, char *out, size_t out_size);
void ft_error(const char *msg);

#endif // HELPER_H