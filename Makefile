CC = gcc
CFLAGS = -Wall -Wextra -pedantic

all: a.out b.out c.out d.out Scheduler

a.out: job_a.c
	$(CC) $(CFLAGS) job_a.c -o a.out

b.out: job_b.c
	$(CC) $(CFLAGS) job_b.c -o b.out

c.out: job_c.c
	$(CC) $(CFLAGS) job_c.c -o c.out

d.out: job_d.c
	$(CC) $(CFLAGS) job_d.c -o d.out

e.out: job_e.c
	$(CC) $(CFLAGS) job_e.c -o e.out

Scheduler: Scheduler.c
	$(CC) $(CFLAGS) Scheduler.c -o Scheduler

clean:
	rm -f a.out b.out c.out d.out Scheduler

