CC = gcc
CFLAGS = -Wall -Wextra -Iinclude -Ilib/datastructures/include
LDFLAGS = -lz

# OpenSSL paths differ by platform
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
    CFLAGS += -I$(shell brew --prefix openssl)/include
    LDFLAGS += -L$(shell brew --prefix openssl)/lib
endif
LDFLAGS += -lcrypto

SRC = src/main.c src/sender.c src/receiver.c src/discovery.c src/helper.c src/crypto.c
LIB_SRC = lib/datastructures/src/vector.c \
          lib/datastructures/src/linked_list.c \
          lib/datastructures/src/hashmap.c \
          lib/datastructures/src/hashset.c

all:
	$(CC) $(CFLAGS) $(SRC) $(LIB_SRC) $(LDFLAGS) -o ft

clean:
	rm -f ft

.PHONY: all clean
