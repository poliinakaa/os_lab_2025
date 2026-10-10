CC=gcc
CFLAGS=-I. -Wall -Wextra -std=c11

.PHONY: all clean test

all: sequential_min_max parallel_min_max run_sequential tests

sequential_min_max: utils.o find_min_max.o sequential_min_max.c utils.h find_min_max.h
	$(CC) $(CFLAGS) -o sequential_min_max find_min_max.o utils.o sequential_min_max.c

parallel_min_max: utils.o find_min_max.o parallel_min_max.c utils.h find_min_max.h
	$(CC) $(CFLAGS) -o parallel_min_max utils.o find_min_max.o parallel_min_max.c

run_sequential: run_sequential.c
	$(CC) $(CFLAGS) -o run_sequential run_sequential.c

tests: tests.c find_min_max.o find_min_max.h utils.h
	$(CC) $(CFLAGS) -o tests tests.c find_min_max.o

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -o utils.o -c utils.c

find_min_max.o: find_min_max.c utils.h find_min_max.h
	$(CC) $(CFLAGS) -o find_min_max.o -c find_min_max.c

test: tests
	./tests

clean:
	rm -f utils.o find_min_max.o sequential_min_max parallel_min_max run_sequential tests
