CC = gcc
CFLAGS = -Wall -Wextra -Iinclude

SRC = src/main.c src/sender.c src/receiver.c src/discovery.c src/helper.c

all:
	$(CC) $(CFLAGS) $(SRC) -o ft

clean:
	rm -f ft

.PHONY: all clean
