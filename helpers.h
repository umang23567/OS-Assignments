// guards
#ifndef HEADER_H
#define HEADER_H

#include <stdio.h>      // For input/output functions 
#include <stdlib.h>     // For general utilities
#include <unistd.h>     // For system calls
#include <sys/wait.h>   // For process waiting
#include <sys/time.h>   // For getting time
#include <string.h>     // For string manipulation functions
#include <fcntl.h>      // For working with file descriptors
#include <errno.h>      // For error handling
#include <time.h>       // For working with timestamps
#include <signal.h>     // For handling signals

#define MAX_INPUT_SIZE 256
#define MAX_ARGS 50

typedef struct 
{
    char command[MAX_INPUT_SIZE];
    double duration;
    pid_t pid;
} CommandHistory;

extern CommandHistory history[MAX_INPUT_SIZE];
extern int history_count;

void print_prompt();
void add_to_history(const char* cmd, pid_t pid, double duration);
void print_history();
void create_process_and_run(char* command);

#endif // HEADER_H
