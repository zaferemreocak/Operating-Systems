# Operating Systems

## Project #1
    This programming assignment is to write a simple shell.
The shell accepts user commands and then executes each command in a separate process.

The shell supports 3 different types of commands:
-- System commands,
-- Built-in commands,
-- Pipe operator.

### System Commands
  It will take the command as input and will execute that in a new process. When the program
  gets the program name, it will create a new process using fork system call, and the new
  process (child) will execute the program.

### Built-in Commands
  - cd <directory>: Change the current directory to <directory>.
  - clr: Clear the screen.
  - dir: Print the current working directory.
  - wait: Described for background processes.
  - hist: This command is for maintaining a history of commands previously issued.
  - exit: Terminate your shell process.
  
### Pipe Operator
For a pipe in the command line, take care of connecting stdout of the left command to stdin of the command following the "|".

## Project #2
    This project has been implemented using Python's low level "_thread" API.
    Usage:  python project_2.py ${location_of_input_file}
Modification of a simple text file such that changing into capital, putting dashes between words via threads. Prints out total number of lines to stdout and creates a log file contains whole operation history of all threds.
There are 4 different queues between each thread's function to put and get a line which is read from input text file. INTERRUPT_SIG is a termination signal that is put into queues when end of file/operation. A thread received this signal exits infinite loop and thread terminates.
