#include <stdio.h>
#include <stdint.h>

FILE *fp = fopen("testfile.bin", "rb");
char buf[32768];
size_t total = 0, n;
while ((n = fread(buf, 1, sizeof(buf), fp)) > 0) total += n;
printf("read %zu bytes\n", total);
