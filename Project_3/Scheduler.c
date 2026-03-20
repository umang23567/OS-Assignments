#include <stdio.h>
#include "dummy_main.h"

// constant values
const int DEFAULT_PRIORITY=1;
const int MIN_PRIORITY=1;
const int MAX_PRIORITY=4;
const int MAX_INPUT_SIZE=256;
const int MAX_PROCESSES=1000;

// global variables
int NCPU=1,TSLICE=100;   

// Process structure
struct process {   

    char* command;                      // Command to execute
    int pid;                            // Process ID
    int priority;                       // Process priority
    int state;                          // 0=new, 1=running, 2=completed

    struct process* next;               // Pointer to the next process in queue

    double wait_time;                   // Total wait time for the process
    struct timeval start_time;          // Start time of the process
    struct timeval slice_time;          // End time of each slice
    double completion_time;             // Completion time of the process
    int run_count;                      // Count of how many times the job was executed
};

// Shared memory structure
struct shared_memory {

    struct process* front;                                      // Front of queue
    struct process* rear;                                       // Rear of queue
    sem_t sem;                                                  // Semaphore for scheduling

    int np;                                                     // Number of processes in queue
    int completed;                                              // Count of completed processes
    struct process completed_procs[MAX_PROCESSES];              // Stores completed processes
};

// Shared memory pointer
struct shared_memory* q;          


// function declarations
void initialize_queue();
static void signal_handler(int signum);
int get_priority(char* command);
void enqueue_process(char* command, int priority);
void dequeue_process(int pid);
char* read_command();
int create_process(struct process* head);
void start_scheduler();
void print_statistics();
void call_scheduler();
void shell_loop();


// function to create a queue and initialize semaphhore
void initialize_queue() 
{
    q->front=NULL;
    q->rear=NULL;
    q->np=0;
    q->completed = 0;

    sem_init(&(q->sem), 1, 0);        // Semaphore initialized to 0
}

static void signal_handler(int signum) 
{
    if (signum==SIGINT) exit(0);
}

// function to extract priority value (if exists) from command 
int get_priority(char* command) 
{
    char *job_name=strtok(command, " ");
    char *priority_str=strtok(NULL," ");
    int priority=DEFAULT_PRIORITY;
    if (priority_str) 
    {
        priority = atoi(priority_str);                         
        if (priority<MIN_PRIORITY || priority>MAX_PRIORITY) 
        {
            fprintf(stderr, "Priority out of range, Using default priority 1.\n");
            priority=DEFAULT_PRIORITY; 
        }
    }
    else
    {
        fprintf(stderr, "Priority not given, Using default priority 1.\n");
    }
    return priority;
}

char* read_command()
{
    char* input=(char*)malloc(MAX_INPUT_SIZE * sizeof(char));           // Allocate memory
    if (input==NULL) 
    {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }
    if (fgets(input,MAX_INPUT_SIZE, stdin)==NULL) {
        perror("Error in reading line");
        exit(EXIT_FAILURE);
    }

    size_t l=strlen(input);
    if (l>0 && input[l-1]=='\n') 
    {
        input[l-1] = '\0';
    } 
    return input;
}

// Function to create a new process
int create_process(struct process* head) 
{
    char* command=head->command;
    int pid=fork();               // Create a new process
    
    if (pid==0)               // Child process
    { 
        struct process* p=malloc(sizeof(struct process));
        if (p==NULL) 
        {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }
 
        execlp("/bin/sh", "sh", "-c", command, NULL); 

        perror("Execution failed");
        exit(EXIT_FAILURE);             // EXIT_FAILURE to exit 
    }
    else if (pid<0)     // Fork failed
    { 
        perror("Fork failed");
        exit(EXIT_FAILURE);             // EXIT_FAILURE to exit
    }

    return pid;         // Return child PID to the parent
}


// Enqueue process (based on priority)
void enqueue_process(char* command, int priority) {
    struct process* p = (struct process*)malloc(sizeof(struct process));
    p->command=strdup(command);
    p->pid=-1;                    
    p->priority=priority;
    p->state=0;                   // new process
    p->next=NULL;
    p->wait_time=0;               
    p->completion_time=0;

    // Store start time when enqueued
    gettimeofday(&p->start_time,NULL);       
    gettimeofday(&p->slice_time,NULL);


    if (q->front==NULL) 
    {
        q->front = q->rear = p;
    } 
    else 
    {
        struct process* current = q->front;
        struct process* prev = NULL;

        // Insert in priority order
        while (current != NULL && current->priority <= p->priority) {
            prev = current;
            current = current->next;
        }

        if (prev==NULL) 
        {
            p->next=q->front;
            q->front=p;
        } else {
            p->next=current;
            prev->next=p;
        }


        if (current==NULL) 
        {
            q->rear=p;
        }
    }

    q->np++;  // Increment process count
}

// function to dequeue process 
void dequeue_process(int pid)
{
    struct process* current=q->front;
    struct process* previous=NULL;

    while (current!=NULL && current->pid!=pid) {
        previous=current;
        current=current->next;
    }

    if (current==NULL) 
    {
        printf("Process with PID %d not found.\n", pid);
        return;
    }

    if (previous == NULL) 
    {

        q->front = current->next;
        if (q->front == NULL) {
     
            q->rear = NULL;
        }
    } 
    else 
    {
        previous->next=current->next;
        if (current==q->rear) 
        {

            q->rear=previous;
        }
    }
    q->np--;
    free(current->command); 
    free(current);   
}

// Scheduler function (RR)
void start_scheduler() 
{
    int status;
    // printf("Scheduler started (PID: %d)\n",getpid());
    while (q->np) 
    {
        // If queue is empty, wait for new jobs
        if (q->front==NULL) 
        {
            printf("Queue is empty. Waiting for new jobs...\n");
            sem_wait(&(q->sem));                  // Wait for jobs
            continue;                                       // Skip to the next iteration 
        }

        int proc_count=(NCPU>q->np)?NCPU:q->np;          // Min of cpu and rem procs
        struct process* head=q->front;

        // Start running processes
        for (int cnt=0;cnt<proc_count && head!=NULL;cnt++) 
        {
            if (head->state == 0)           // New process, start it
            { 
                struct timeval end_time;
                gettimeofday(&end_time,NULL);
                head->wait_time+=(end_time.tv_sec-head->start_time.tv_sec)*1000;
                head->wait_time+=(end_time.tv_usec-head->start_time.tv_usec)/1000;
                head->pid=create_process(head);
                // printf("Starting process: '%s', pid: %d\n", head->command, head->pid);
                head->state=1;          // Mark as running
            } 
            else if (head->state==1)          // Already running, continue it
            { 
                // printf("Continuing process with pid: %d\n", head->pid);
                struct timeval end_time;
                gettimeofday(&end_time,NULL);
                head->wait_time+=(end_time.tv_sec-head->slice_time.tv_sec)*1000;
                head->wait_time+=(end_time.tv_usec-head->slice_time.tv_usec)/1000;
                if (kill(head->pid, SIGCONT)==-1) 
                {
                    perror("Error resuming process");
                    exit(EXIT_FAILURE);
                }
            }
            head=head->next;
        }

        // Wait for the time slice to expire
        sem_post(&q->sem);
        usleep(TSLICE*1000);                        // uSleep for TSLICE*1000 microseconds
        sem_wait(&q->sem);
        head=q->front;
        // Pause processes after time slice
        for (int cnt=0;cnt<proc_count && head!=NULL;cnt++) 
        {
        // printf("\n%d processes, processing %d, state: %d\n", q->np, cnt, head->state);

            if (head->state == 1)               // handle running processes 
            { 
                int res=waitpid(head->pid, &status,WNOHANG);
                if (res==0)           // Process is still running
                { 
                    // printf("Pausing process\n");
                    int result = kill(head->pid, SIGSTOP);
                    if (result == -1) 
                    {
                        perror("Error pausing process");
                        exit(EXIT_FAILURE);
                    }
                    gettimeofday(&head->slice_time,NULL);

                } 
                else if (res==head->pid) 
                { 
                    head->state = 2;            // Mark as completed
                    
                    struct timeval end_time;
                    gettimeofday(&end_time,NULL);
                    head->completion_time=(end_time.tv_sec-head->start_time.tv_sec)*1000;
                    head->completion_time+=(end_time.tv_usec-head->start_time.tv_usec)/1000;
                    // printf("Process with PID: %d has completed.\n",head->pid);
            
                    struct process* proc=malloc(sizeof(struct process));
                    if (proc==NULL) 
                    {
                        perror("Memory allocation failed");
                        exit(EXIT_FAILURE);
                    }

                    memcpy(proc, head, sizeof(struct process)); // Copy the current process
                    q->completed_procs[q->completed] = *proc; // Add to completed processes
                
                    q->completed++; 
                
                    dequeue_process(head->pid); 
                } 
                else 
                {
                    head=head->next; 
                }
            } 
            else
            {
                head=head->next;
            }

        }
        sem_post(&q->sem);
        
    }

}

void call_scheduler()
{
    pid_t pid=fork();
    if (pid<0) 
    {
        perror("Failed to start scheduler");
    } 
    else if (pid==0) 
    {
        start_scheduler();          // Run scheduler in child process
        exit(0);                    // Exit the child process after scheduler ends
    }
}

void shell_loop() 
{
    while (1) 
    {
        char* command;
        printf("Shell$ ");
        command=read_command();

        // Handle 'exit' command 
        if (strcmp(command,"exit")==0) 
        {
            printf("Exiting SimpleShell......\n");
            print_statistics();  // Print job statistics on exit
            break;
        }

        // Start the scheduler
        else if (strcmp(command,"start")==0) 
        {
            call_scheduler();
        }

        // Handle job submission
        else if (strncmp(command,"submit ",7)==0) 
        {
            char job_command[MAX_INPUT_SIZE];
            snprintf(job_command, MAX_INPUT_SIZE,"%s",command+7);       // Skip "submit "

            int priority = get_priority(command+7);  

            // Enqueue process
            enqueue_process(job_command,priority);
            printf("Job submitted: %s with priority %d\n",job_command,priority);
            sem_post(&(q->sem));                               // Signal the scheduler to wake up
        }   
        
        else 
        {
            printf("Unknown command, Available commands: submit, start, exit\n");
        }
    }

    return;
}

void print_statistics() 
{
    // Wait for all processes to finish
    while (q->np>0) 
    {
        usleep(10000);          // Sleep briefly to prevent busy waiting
    }


    printf("Job Statistics:\n");
    for (int i=0;i<q->completed;i++) 
    {
        struct process* job = &q->completed_procs[i];
        printf ("Job: %s PID: %d, Completion Time: %.2f ms, Wait Time: %.2f ms\n",
            job->command, job->pid, job->completion_time/100, job->wait_time/100) ;
    }
}


int main(int argc, char* argv[]) 
{
    if (argc<3) 
    {
        fprintf(stderr,"Usage: %s <NCPU> <TSLICE>\n",argv[0]);
        return EXIT_FAILURE;
    }

    int NCPU=atoi(argv[1]);
    int TSLICE=atoi(argv[2]);

    if (NCPU<=0 || TSLICE<=0) 
    {
        fprintf(stderr, "NCPU and TSLICE must be positive.\n");
        return EXIT_FAILURE;
    }

    struct sigaction sig;
    memset(&sig,0,sizeof(sig));
    sig.sa_handler=signal_handler;
    sig.sa_flags=0;
    sigaction(SIGINT,&sig,NULL);


    // create shared memory
    int shm_fd=shm_open("shmfile", O_CREAT | O_RDWR, 0666);
    if (shm_fd==-1) 
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    // Set size of shared memory
    int shm_size=sizeof(struct shared_memory);
    ftruncate(shm_fd, shm_size);

    // Map shared memory
    q=(struct shared_memory*)mmap(NULL, shm_size, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (q==MAP_FAILED) 
    {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // Initialize queue
    initialize_queue();

    // Start shell loop
    printf("Starting shell...\n");
    shell_loop();

    // Clean up shared memory
    if (munmap(q, shm_size)==-1) 
    {
        perror("munmap");
    }
    close(shm_fd);
    sem_destroy(&q->sem);
    shm_unlink("shmfile");

    return 0;
}