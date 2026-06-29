#ifndef DISCOVERY_H
#define DISCOVERY_H

#include <stdio.h>
int discovery_listen(int tcp_fd);
int discovery_find(char *ip_out, size_t ip_size);


#endif // DISCOVERY_H
