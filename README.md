# Process Scheduler

## Overview

 This is a process scheduler that utilizes a priority-based queue and round-robin scheduling.
 It is designed to allow users to submit multiple jobs (processes) with assigned priorities,
 which can then be scheduled and managed by the system. Processes are handled in a shared
 memory queue, and users can control when the scheduler starts processing.


## Key Features

### Process Scheduling

 - **Priority-based**: Processes are assigned priorities ranging from 1 (lowest) to 4 (highest).
    The scheduler dequeues processes based on priority levels.

 - **Round-Robin Execution**: Within each priority level, round-robin scheduling is applied.

 - **Shared Memory Queue**: Processes are maintained in a shared memory queue allowing
    multiple commands to be submitted and managed efficiently.



### Multiple Submissions

 The system supports multiple job submissions via the `submit` command. Once all jobs are
 submitted, the `start` command initializes the scheduling of all jobs as a batch. 

### Job Statistics
 Once all jobs are completed, the scheduler generates statistics that display the completion
 and wait times of each job. These metrics allow users to analyze the effect of priority
 levels on scheduling efficiency.

## Implementation Details

### 1. Core Components and Structure

 - **Process Queue**: A shared memory queue is used to manage processes. Each job (process) has attributes like PID, priority, completion time, and wait time, allowing for efficient scheduling and tracking.
 
 - **Priority Levels**: The scheduler supports four levels of priority, with 4 being the highest and 1 being the lowest. Higher-priority jobs are scheduled before lower-priority ones.
 
 - **Round-Robin Scheduling**: For processes within the same priority level, a round-robin approach is used with a specified time slice (`TSLICE`). This ensures fair execution of processes within each priority band.
 
 - **Semaphore and Shared Memory**: The implementation uses semaphores to control access to shared resources and shared memory for inter-process communication. This ensures that multiple commands can interact with the queue without conflicts.

### 2. Command Structure and Handling

 - **`submit` Command**: 
    - Each job submission command adds a new job to the shared memory queue.
    - The submitted job includes details such as the process command and priority level.
    - A new job is enqueued in the appropriate priority band based on the specified priority.

 - **`start` Command**:
    
     - Triggers the `start_scheduler()` function, which begins the scheduling loop.
    
     - It starts a new process to handle all enqueued jobs and operates in batch mode, iterating over each job in priority order.
    
     - Without the `start` command, the scheduler does not process jobs, allowing multiple jobs to be submitted before starting them as a group.

 - **`exit` Command**:
    
     - Ends the shell loop, completing the program and outputting the job statistics.
    
     - Displays wait and completion times for each job, demonstrating the effect of their priorities.

### 3. Scheduler Execution (start_scheduler Function)

 - The `start_scheduler()` function processes all enqueued jobs based on priority. It involves:
    
     - **Priority Queue Processing**: Jobs are dequeued from the highest priority first. For each priority level, jobs are managed in a round-robin fashion.
     
     - **Fork and Exec**: For each job, a new process is created. `fork()` generates the child process, and `exec` runs the specified command.
     
     - **Time Slice Control**: If the time slice (`TSLICE`) expires before the job completes, it is paused using `kill()` and re-added to the end of its priority queue for the next round.
     
     - **Completion Check**: If a job completes within its time slice, it’s removed from the queue and marked as completed, with completion and wait times updated accordingly.

### 4. Statistics Collection

 - Once all jobs are completed, the program calculates:
   
    - **Completion Time**: The time taken for each job to complete.
   
    - **Wait Time**: The time each job spent waiting in the queue before execution.
 
 - These statistics help users understand the impact of job priority on execution and waiting times.
 

## Process Flow

 1. **Shell Loop**: The main loop receives commands in the form of `submit`, `start`, and `exit`.

 2. **Command Parsing**: The shell parses each command and extracts the job name and priority.

 3. **Enqueueing**: Jobs are enqueued based on priority levels. Higher priority jobs are placed
    at the beginning of the queue.

 4. **Scheduler Execution**: The `start` command invokes the `start_scheduler()` function in
    a new process, running the scheduler loop which handles all enqueued jobs using a round-robin
    approach with priority.

 5. **Process Control**: Each job is executed, paused, or resumed based on the round-robin timeslice.


## Running the Program

### Compiling

make

.scheduler <NCPU> <TSLICE>

### Example

make

./scheduler 2 100


### Commands

 Shell$ start
 
 Starting scheduler...
 
 Shell$ exit


## Conclusion
 
 The process scheduler efficiently manages jobs with varying priorities, allowing multiple
 job submissions and batch processing. Priority-based scheduling and round-robin execution
 ensure a fair distribution of resources, while job statistics provide insights into system
 performance.

## Contribution
 
 Umang Aggarwal - Code
 Vinit Kumar - Readme
