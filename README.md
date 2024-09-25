Simple Shell - README

Overview:-
This project implements a basic Unix shell in C, called simple-shell. The shell allows users to execute commands, either sequentially or in parallel (background processes), handle piping between commands, and maintain an execution history with information such as the process ID (PID) and execution duration.

Features:-
Basic Command Execution: Supports standard Unix commands like ls, pwd, echo, etc.
Pipes Support: Allows chaining multiple commands using pipes (|), enabling the output of one command to be used as input for another.
Background Execution: Commands can be executed in the background by appending &.
Command History: The shell maintains a history of executed commands, showing:
The command string.
The process ID (PID).
The execution duration in milliseconds.
Built-in Commands:
history: Displays the history of executed commands.
exit: Terminates the shell.

How to Use:-
1. Compilation: To compile the program, ensure that you have gcc or any C compiler installed. Run the following command: gcc -o shell shell.c
2. Running the Shell: Once compiled, run the shell by executing the following command:./simple-shell
3. Executing Commands: Enter any Unix command (e.g., ls, ls/home, etc) to execute it.
4. Error Handling: If an invalid command is entered, the shell will print an error message: No such file or directory.

Code Structure:-
1. print_prompt(): Displays the shell prompt simple-shell>.
2. add_to_history(): Adds a command to the execution history, including its process ID and execution time.
3 print_history(): Prints the history of executed commands.
4. parse_space(): Splits a command string into arguments separated by spaces.
5. parse_pipes(): Splits a command into multiple commands based on pipes (|).
6. setup_pipes(): Sets up the pipes necessary for executing piped commands.
7. redirect_input() / redirect_output(): Redirects the input and output to/from pipes or files.
8. create_process_and_run(): Executes the command, handles pipes, forks processes, and records execution time for history.

Documentation:-
Umang Aggarwal-
Vinit Kumar- Bonus part, readme

GitHub link: https://github.com/vinit23599/OS-Project-2



