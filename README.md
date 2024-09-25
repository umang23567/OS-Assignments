Simple Shell - README

Overview:
The code implements a basic shell that can:
Execute Unix commands.
Handle multiple commands with pipes (|).
Run commands in the background (&).
Maintain a history of executed commands, including process ID and execution time.

Key Concepts and Components-

 Libraries :
  stdio.h, stdlib.h, unistd.h, sys/wait.h, string.h, fcntl.h, errno.h, time.h, signal.h, sys/time.h
  These libraries provide system-level functionalities such as creating processes (fork()), executing commands (execvp()), managing pipes,       
  and handling input/output.

 Macros:
  MAX_INPUT_SIZE: Defines the maximum length of an input command (1024 characters).
  MAX_ARGS: Sets the maximum number of arguments a command can have (100).


Data Structures-

 Command_History
  The Command_History structure is used to store details about each executed command. It contains:
  command: The command string entered by the user.
  timestamp: The time the command was executed.
  pid: The process ID (PID) of the command.
  duration: The time taken for the command to execute, in milliseconds.

Core Functions-
 1. print_prompt()
  Displays the shell prompt simple-shell>.
  Called before every user input to prompt the user for the next command.

 2. add_to_history()
  Adds executed commands to the history[] array.
  Stores the command string, process ID, and execution duration (in milliseconds).
 
 3. print_history()
  Prints the history of previously executed commands.
  For each command in the history, it displays:
  The command string.
  The process ID.
  The duration of the command in milliseconds.
 
 4. parse_space()
  Splits the command string into arguments using spaces as delimiters.
 
 5. parse_pipes()
  Splits the input command into multiple commands, separated by pipes (|).
 
 6. setup_pipes()
  Creates the necessary pipes for executing multiple commands.
  Each pipe allows inter-process communication, where the output of one command can serve as the input for another.
 
 7. redirect_input() / redirect_output()
  redirect_input(): Redirects the input (stdin) from a pipe or file descriptor.
  redirect_output(): Redirects the output (stdout) to a pipe or file descriptor.
 
 8. create_process_and_run()
  The core function responsible for executing commands:
  Checks if the command is running in the background (by checking for &).
  Measures the execution time of each command.
  Handles built-in commands such as exit (to quit the shell) and history (to view the command history).
  If the command contains pipes, it splits the command and forks a child process for each command segment.
  Uses execvp() to execute the command in the child process.
  Waits for foreground processes to complete, but allows background processes to run without blocking the shell.
  Logs the command in the history after execution.
 
 9. main()
  The main loop of the shell, which runs indefinitely until the user types exit.
  Repeatedly prints the prompt, waits for user input, and calls create_process_and_run() to execute the command.


Execution Flow-

 Main Loop
  The shell runs an infinite loop that:
  Displays the prompt (simple-shell>).
  Reads input from the user.
  Sends the command to create_process_and_run() for execution.
 
 Command Execution
  Built-in Commands: 
   If the command is exit, the shell terminates.
   If the command is history, the shell prints the history of all executed commands.
  
  Command with Pipes: 
   If pipes (|) are present, the command is split, and each segment is executed in separate processes.
  
  Background Execution: 
   If the command ends with &, it runs in the background without blocking the shell.

  History Tracking: 
   The shell logs every executed command in the history[] array, including its process ID, timestamp, and execution     
   duration.


Error Handling-

  execvp(): If the command fails (e.g., it doesn’t exist), an error message is printed (execvp failed).

  Forking and Pipes: If creating a child process (fork()) or setting up a pipe fails, appropriate error messages are printed.

Timing and History-
 Execution Time: The duration of each command is measured using gettimeofday() and stored in the history in milliseconds.
 Command History:
 A history of executed commands is maintained.

Limitations:- In this shell, we currently can't use these commands-

 Built-in Shell Commands: Commands like cd, exit, and export: These commands are built into most shells (e.g., Bash) and do not spawn        
external processes. In your custom shell, these commands will not work as expected unless you implement them yourself.

 Commands Requiring Interactive Input: Commands like ssh, su, passwd: These commands require interactive input (e.g., passwords), and  handling them in a custom shell can be difficult. These commands might run but may not behave as expected in terms of interaction with the user.
 
 I/O Redirection: This shell does not currently support file redirection using >, <, or >>.
 
 Pipelines with Built-in Commands: cd in Pipelines: If you use cd in a pipeline like cd /path | ls, the behavior will not be correct   because cd affects only the current shell process, and piping it to another command creates a separate child process where the directory change won’t persist.



Contribution:-
Umang Aggarwal- execution and testing
Vinit Kumar-  documentation, parsing and bonus

GitHub link: https://github.com/vinit23599/OS-Project-2



