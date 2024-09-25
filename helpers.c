#include "helpers.h"

CommandHistory history[MAX_INPUT_SIZE];
int history_count = 0;

void print_prompt() 
{
    printf("shell> ");
}

void add_to_history(const char* cmd,pid_t pid,double duration) 
{
    if (history_count<MAX_INPUT_SIZE) 
    {
        history[history_count].duration=time(NULL);
        history[history_count].pid=pid;
        strncpy(history[history_count].command,cmd,MAX_INPUT_SIZE);
        history_count++;
    }
}

void print_history() 
{
    for (int i=0;i<history_count;i++) 
    {
        printf("[%d] %s (PID: %d) Duration: %.10f seconds \n", i+1,history[i].command,history[i].pid,history[i].duration);
    }
}

