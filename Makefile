CC=gcc
CFLAGS=-g -pedantic -std=gnu17 -Wall -Werror -Wextra

.PHONY: all clean

all: nyush

nyush: nyush.o io_redirect.o pipeline.o jobs.o
	$(CC) $(CFLAGS) -o nyush nyush.o io_redirect.o pipeline.o jobs.o

nyush.o: nyush.c io_redirect.h pipeline.h jobs.h
	$(CC) $(CFLAGS) -c nyush.c

io_redirect.o: io_redirect.c io_redirect.h
	$(CC) $(CFLAGS) -c io_redirect.c

pipeline.o: pipeline.c pipeline.h io_redirect.h
	$(CC) $(CFLAGS) -c pipeline.c

jobs.o: jobs.c jobs.h
	$(CC) $(CFLAGS) -c jobs.c

clean:
	rm -f *.o nyush
