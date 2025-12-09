CC=gcc
SRCS=main.c pagerank.c graph.c thr_pool.c
OBJS=$(SRCS:%.c=./bin/%.o)
CFLAGS=-g -O0

all: pre-build $(OBJS)
	$(CC) $(OBJS) -o ./bin/main

pre-build:
	mkdir -p ./bin

./bin/%.o: ./%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf ./bin
