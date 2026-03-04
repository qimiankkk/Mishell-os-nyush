CC=gcc
CFLAGS=-g -pedantic -std=gnu17 -Wall -Werror -Wextra

.PHONY: all clean

all: nyush

nyush: nyush.o io_redirect.o
	$(CC) $(CFLAGS) -o nyush nyush.o io_redirect.o

nyush.o: nyush.c io_redirect.h
	$(CC) $(CFLAGS) -c nyush.c

io_redirect.o: io_redirect.c io_redirect.h
	$(CC) $(CFLAGS) -c io_redirect.c

clean:
	rm -f *.o nyush
