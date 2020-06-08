MYSHELL DOCUMENTATION

A. Description

myshell is a c program with the function of stimulating the UNIX/LINUX shell (or command line intepreter), each
user can send commands to the OS then the OS can respond to the user.

B. Executing

The user can open myshell by either typing ./myshell, which opens the
program's interactive mode, or ./myshell batchfile (.txt file), which opens the batch mode.

C. Functionality

myshell has the following functionalities:

1. Interactive read/execute loop:  myshell will continuously take in user commands until the user type in the exit command.
   
   The prompt of myshell will display the folder the user is currently in.

2. Batch mode (reading from file): While in batch mode, myshell will read and execute the commands from a text file.

3. Internal commands: My shell will support the following internal commands: 

	a. cd <directory> - Change the current directory to <directory>. If the <directory> argument is not present,
	   report the current directory. If the directory does not exist, display the error message to the user.
	   Also change the PWD environment variable. 
	b. clr - Clear the screen.
	c. dir <directory> - List the contents of <directory>.
	d. environ - List all the environment strings.
	e. echo <comment> - Display <comment> on the screen followed by a new line.
	f. help - Display the user manual using the more filter. 
	g. pause - Stop shell operation until 'Enter' is pressed.
	h. quit - Quit the shell.


4. Program invocation: myshell will treat all other commands as program invocation and try to execute
   it using fork() and exec() (create child process). 

5. I/O redirection: myshell will support I/O redirection on either or both stdin and/or stdout.
   I/O redirection is recognized by user typing in ">", "<", ">>", etc.
   , and will be supported for both program invocation and internal commands.

6. Background execution of programs: If the user types in the ampersand sign (&) at the end of the command line
   , myshell will immediate return to taking in commands, while the command in the background is still being processed.

7. Pipe implementation: myshell will implement the Unix pipe to redirect I/O.

D. Implementation

To implement the program's interactive loop,  the codes in the main function can be executed with a while loop, and the system will repeated
until the user call the quit command. In batch mode, the program will continously read and executes lines of commands until end of file is reached.

The code for internal commands, program I/O, and file execute/read/write will be implemented using the appropriate system calls.

To process user commands, the functions from string.h library can be used, such as strtok() (used to tokenized the string into separate words).
An implementation of sets (data structure) may be used to check if the command contains members of a set of internal commands.
All other commands are treated as program invocation and will be handled accordingly (using fork() and exec().
Proper error messages can be displayed using the errno.h library and the stderr file descriptor.

To handle I/O redirection, system calls such as fork(), dup(), and dup2(), can be used along with the proper file descript
