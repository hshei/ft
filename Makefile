CFLAGS = -Wall -Wextra -Iinclude -lz -Ilib/datastructures/include

SRC = src/main.c src/sender.c src/receiver.c src/discovery.c src/helper.c
LIB_SRC = lib/datastructures/src/vector.c \
          lib/datastructures/src/linked_list.c \
          lib/datastructures/src/hashmap.c \
          lib/datastructures/src/hashset.c

all:
	$(CC) $(CFLAGS) $(SRC) $(LIB_SRC) -o ft
