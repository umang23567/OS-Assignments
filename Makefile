# Makefile for compiling fib, sum, and loader programs

# Compiler and flags
CC = gcc
CFLAGS = -m32 -no-pie -nostdlib

# Targets
all: fib sum loader

fib: fib.c
	$(CC) $(CFLAGS) -o fib fib.c

sum: sum.c
	$(CC) $(CFLAGS) -o sum sum.c

loader: loader.c
	$(CC) -m32 -o loader loader.c

clean:
	-@rm -f fib sum loader
