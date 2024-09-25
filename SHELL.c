#include <stdio.h>      // For input/output functions 
#include <stdlib.h>     // For general utilities
#include <unistd.h>     // For system calls
#include <sys/wait.h>   // For process waiting
#include <string.h>     // For string manipulation functions
#include <fcntl.h>      // For working with file descriptors
#include <errno.h>      // For error handling
#include <time.h>       // For working with timestamps
#include <sys/time.h>   // For working with high resolution time values
#include <signal.h>     // For handling signals

#define MAX_INPUT_SIZE 1024
#define MAX_ARGS 100

typedef struct 
{
    char command[MAX_INPUT_SIZE];
    time_t timestamp;
    pid_t pid;
    double duration;
} CommandHistory;

CommandHistory history[MAX_INPUT_SIZE];
int history_count = 0;

void print_prompt() {
    printf("simple-shell> ");
}

void add_to_history(const char* cmd, pid_t pid, double duration) 
{
    if (history_count < MAX_INPUT_SIZE) 
    {
        history[history_count].timestamp=time(NULL);
        history[history_count].pid=pid;
        history[history_count].duration=duration;
        strncpy(history[history_count].command, cmd, MAX_INPUT_SIZE);
        history_count++;
    }
}

void print_history() 
{
    for (int i=0;i<history_count;i++) 
    {
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", localtime(&history[i].timestamp));
        printf("[%d] %s (PID: %d, Duration: %.2f ms)\n", i + 1, history[i].command, history[i].pid, history[i].duration);
    }
}

void parse_space(char* command, char* args[]) 
{
    char* cmd_token=strtok(command, " ");
    int j=0;
    while (cmd_token!=NULL) 
    {
        args[j++]=cmd_token;
        cmd_token=strtok(NULL," ");
    }
    args[j]=NULL;
}

// Function to parse commands based on pipes
int parse_pipes(char* command, char* commands[]) 
{
    char* token=strtok(command,"|");
    int num_commands=0;

    while (token!=NULL && num_commands<MAX_ARGS) 
    {
        commands[num_commands++]=token;
        token=strtok(NULL,"|");
    }
    commands[num_commands]=NULL;

    return num_commands;
}

// Function to setup pipes
void setup_pipes(int pipefd[], int num_commands) 
{
    for (int i=0;i<num_commands-1;i++) 
    {
        if (pipe(pipefd+i*2)==-1) 
        {
            perror("pipe failed");
            exit(EXIT_FAILURE);
        }
    }
}

// Function to handle input redirection
void redirect_input(int input_fd) 
{
    if (dup2(input_fd, STDIN_FILENO)==-1)
    {
        perror("dup2 input failed");
        exit(EXIT_FAILURE);
    }
}

// Function to handle output redirection
void redirect_output(int output_fd) {
    if (dup2(output_fd, STDOUT_FILENO)==-1) 
    {
        perror("dup2 output failed");
        exit(EXIT_FAILURE);
    }
}

// function to execute user command
void create_process_and_run(char* command) 
{
    struct timeval start_time,end_time;

    // Check for background execution (BONUS)
    int background=0;
    if (command[strlen(command)-1]=='&') 
    {
        background = 1;
        command[strlen(command) - 1] = '\0'; // Remove the '&' character
    }

    // Start timer
    gettimeofday(&start_time, NULL);

    if (strcmp(command, "exit") == 0) 
    {   // terminate shell
        print_history();
        exit(0);
    }
    if (strcmp(command, "history") == 0) 
    {   // view history
        print_history();
        return;
    }

    char* commands[MAX_ARGS];
    int num_commands = parse_pipes(command,commands);

    int pipefd[2 *(num_commands-1)];
    setup_pipes(pipefd,num_commands);

    pid_t last_pid=-1;

    for (int i=0;i<num_commands;i++) 
    {
        pid_t pid=fork();
        /*
        The child process will execute the command, 
        while the parent process will manage the pipes 
        and wait for child processes to finish
         */
        
        if (pid==0)                 // Child process
        {   
            // if child, we proceed with executing the command

            // Redirect input from previous pipe (for all commands except first)
            if (i != 0) 
            {
                redirect_input(pipefd[2*(i-1)]);
            }

            // Redirect output to next pipe (for all commands except last)
            if (i!=num_commands-1) 
            {
                redirect_output(pipefd[2*i+1]);
            }

            // After redirections set up by child process
            // we close all pipe fd 
            for (int j=0;j<2*(num_commands-1);j++) 
            {
                close(pipefd[j]);
            }

            char* args[MAX_ARGS];
            parse_space(commands[i],args);

            // execute thew command using exec
            if (execvp(args[0],args)==-1) 
            {
                perror("execvp failed");
                exit(EXIT_FAILURE);
            }
        } 
        else if (pid>0)                 // Parent process
        { 
            last_pid=pid;

            // close fds
            if (i!=0) 
            {
                close(pipefd[2*(i-1)]);
            }
            if (i != num_commands-1) 
            {
                close(pipefd[2*i+1]);
            }

            // If running in the background, do not wait for the process
            if (!background) 
            {
                waitpid(pid, NULL, 0);  // Wait for foreground processes
            }
        } 
        else                            // fork returns -1 when failed
        {   
            perror("fork failed");
            exit(EXIT_FAILURE);
        }
    }

    // Close remaining pipe file descriptors 
    for (int i=0;i<2*(num_commands-1);i++) 
    {
        close(pipefd[i]);
    }

    // End timer
    gettimeofday(&end_time, NULL);
    
    // Calculate duration in milliseconds
    double duration =(end_time.tv_sec-start_time.tv_sec)*1000.0;     // seconds to milliseconds
    duration+=(end_time.tv_usec - start_time.tv_usec)/1000.0;        // microseconds to milliseconds

    // Add command to history if it's a foreground process
    if (!background) 
    {
        add_to_history(command, last_pid, duration);
    } 
    else 
    {
        printf("Command running in background with PID: %d\n", last_pid);
    }
}


int main() 
{
    char input[MAX_INPUT_SIZE];
    while (1) {
        print_prompt();
        if (fgets(input, sizeof(input), stdin) == NULL) 
        {
            perror("fgets failed");
            continue;
        }
        
        input[strcspn(input, "\n")] = '\0'; // Removing trailing newline
        
        create_process_and_run(input);
    }
}
