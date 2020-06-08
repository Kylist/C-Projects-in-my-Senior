#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

//myshell only works with command lines of equal or less than 256 chars.
#define MAX_LENGTH 256

//FUNCTIONS PROTOTYPE
void prompt();
void process(char *line);
char **parse(char *line);
int executeNorm(char **command, int builtinNo);
int executeBg(char **command, int builtinNo);
int executeRedir(char **command, char *inputName, char *outputName, int isAppend, int builtinNo);
int executePipe(char ** command1, char ** command2, int builtinNo1, int builtinNo2);
int checkBuiltins(char *args);
int ex_cd(char **args);
int ex_clr(char **args);
int ex_dir(char **args);
int ex_environ(char **args);
int ex_echo(char **args);
int ex_help(char **args);
int ex_pause(char **args);


//list of built-in handler functions
int (*builtins[]) (char **) = {
    &ex_cd,
    &ex_clr,
    &ex_dir,
    &ex_environ,
    &ex_echo,
    &ex_help,
    &ex_pause
};

// list of built-in commands
char *builtin_cmd[] = {
    "cd",
    "clr",
    "dir",
    "environ",
    "echo",
    "help",
    "pause"
};

//Call readme file
char originalDir[MAX_LENGTH];

int main(int args, char* argv[]){
	getcwd(originalDir, sizeof(originalDir));

    if(argv[1] != NULL){    //if reading from a file
        FILE *fp;
        if((fp = fopen(argv[1], "r")) == NULL){
            fprintf(stderr,"Batchfile cannot be opened with name %s.\n",argv[1]);
            return 0;
        }
        char line[MAX_LENGTH];
        //gets lines
			while (fgets(line, MAX_LENGTH, fp)) {
				printf("line: %s", line);
				//replaces new line character with string ending character.
                line[strlen(line)-1] = '\0';
				if (strcmp(line, "quit") == 0) { //quits the shell.
					exit(0);
				}
				process(line);

			}
			fclose(fp);
    } else {
        //Interactive mode
		char line[MAX_LENGTH];
		while (1) {
			prompt(); //Prints prompt.
			fgets(line, MAX_LENGTH, stdin);
            line[strlen(line)-1] = '\0';
            process(line);//processes the command line
		}
	}
	return 0;
}

void prompt(char *str) {
    //Prints prompt.
	char currentDir[MAX_LENGTH];
	getcwd(currentDir, sizeof(currentDir));
	fprintf(stdout,"%s>", currentDir);
}

void process(char *line){
    //Parses command line to array of arguments
    char ** args = parse(line);
    if(args[0]==NULL){
        return;
    }
    //Booleans to switch between functions
    int isBg = 0;
    int isPipe = 0;
    int hasInput = 0;
    int hasOutput = 0;
    int isAppend = 0; //Differentiates between ">" and ">>"

    //In myshell. It executes 1 command/program at a time, or 2 commands/programs when invoking a pipe.
    char *command1[MAX_LENGTH];
    char *command2[MAX_LENGTH]; //Only available when invoking a pipe
    //initializing to catch null cases
    command1[0]=NULL;
    command2[0]=NULL;
    //Initializing input and output file name. Will do a strcpy() if redirection is found in command line
    char inputName[MAX_LENGTH];
    char outputName[MAX_LENGTH];
    inputName[0] = '\0';
    outputName[0] = '\0';

    //Begins processing line
    int i = 0;
    for (; args[i]!=NULL; i++){
        if(strcmp(args[i], "&")==0){
            isBg=1;
            break;
            //if catches the ampersand sign, immediately invokes executeBg(). Contents of the command line after the ampersand sign will be ignored for simplicity
        }
        if(strcmp(args[i], "|")==0){
            //if catches the pipe sign, stops putting arguments into command 1 and starts putting arguments into command 2.
            isPipe=1;
            command1[i] = NULL;
            i++;
            int j = 0;
            while(args[i]!=NULL){
                command2[j] = args[i];
                i++;
                j++;
            }
            command2[j] = NULL;
            break;
        }
        //Catches redirection cases.
        if (strcmp(args[i], ">")==0){
            if(hasOutput){
                printf("%s\n","myshell works with only one output file at a time.");
                return;
            }
            hasOutput = 1;
            command1[i] = NULL;
            //Checks if file name is given
            if (args[++i]==NULL){
                fprintf(stderr,"%s\n","Cannot execute redirection: Output file name is not given.");
                return;
            } else {
            //Set input/output file name
                strcpy(outputName,args[i]);
            }
        } else if (strcmp(args[i], ">>")==0){
            if(hasOutput){
                printf("%s\n","myshell works with only one output file at a time.");
                return;
            }
            hasOutput = 1;
            command1[i] = NULL;
            if (args[++i]==NULL){
                fprintf(stderr,"%s\n","Cannot execute redirection: Output file name is not given.");
                return;
            } else {
                strcpy(outputName,args[i]);
                isAppend = 1;
            }
        } else if (strcmp(args[i], "<")==0){
            if(hasInput){
                printf("%s\n","myshell works with only one output file at a time.");
                return;
            }
            hasInput = 1;
            command1[i] = NULL;
            if (args[++i]==NULL){
                fprintf(stderr,"%s\n","Cannot execute redirection: Input file name is not given.");
                return;
            } else {
                strcpy(inputName,args[i]);
            }
        } else {
            if(hasInput|hasOutput){ //if the function has caught any redirection, passing more arguments or commands will cause an error, except for the case: commands > file1 < file2
                fprintf(stderr,"%s\n", "Too many argument or ambiguous command.");
                return;
            } else {
                command1[i] = args[i];
            }
        }
    }


    if((!isPipe)&&(!hasInput)&&(!hasOutput)){
    command1[i] = NULL;
    }

    if(command1[0]==NULL){
        fprintf(stderr,"%s\n", "Missing or ambiguous command");
        return;
    }
    int builtinNo1 = checkBuiltins(command1[0]);
    //Use builtinNo to decide which functions to call (or program execution) later on
    if(isPipe){
        if(command2[0]==NULL){
            fprintf(stderr,"%s\n", "Cannot execute pipe: missing or ambiguous command 2");
        } else {
            int builtinNo2 = checkBuiltins(command2[0]);
            executePipe(command1,command2, builtinNo1, builtinNo2);
        }
        return;

    } else if(isBg){

        executeBg(command1, builtinNo1);
        return;

    } else if(hasInput|hasOutput){

        executeRedir(command1, inputName, outputName, isAppend, builtinNo1);
        return;

    } else {
        executeNorm(command1, builtinNo1);
    }

    free(args);
    return;
}

char **parse(char *line) {
    char **args;
    char *arg;
    if ((args = malloc(sizeof(char*) * MAX_LENGTH)) == NULL){
        fprintf(stderr,"%s\n","Error in parsing command line");
        exit(EXIT_FAILURE);
    }
    //tokenizes the line and passes the arguments to the array of strings
    arg = strtok(line, " \t\r\n\a");
    int pos = 0;
    while (arg != NULL) {
        args[pos++] = arg;
        arg = strtok(NULL, " \t\r\n\a");
    }

    args[pos] = NULL;
    return args;
}

//Normal execution, either executing a program or run built-in commands
int executeNorm(char **command, int builtinNo){
    if(builtinNo!=-1){
        if((*builtins[builtinNo])(command)!=0){
            fprintf(stderr,"%s %s\n","Failed to execute built-in command:", builtin_cmd[builtinNo]);
            return -1;
        }
        return 0;
    }
    //Program invocation
    pid_t pid;
    if ((pid=fork())==-1) {
        fprintf(stderr,"Fork error.\n");
        return -1;
    } else if (pid == 0) {
        if (execvp(command[0], command) != 0) {
			fprintf(stderr,"Cannot start the program.\n");
		}
		exit(0);
    } else {
		wait(NULL);
	}
    return 0;
}

//Background
int executeBg(char **command, int builtinNo){
    if(builtinNo!=-1){
        if((*builtins[builtinNo])(command)!=0){
            fprintf(stderr,"%s %s\n","Failed to execute built-in command:", builtin_cmd[builtinNo]);
            return -1;
        }
        return 0;
    }
    pid_t pid;
    if ((pid=fork())==-1) {
        fprintf(stderr,"Fork error.\n");
        return -1;
    } else if (pid == 0) {
        if (execvp(command[0], command) != 0) {
			fprintf(stderr,"Cannot start the program .\n");
		}
		exit(0);
    }
    return 0;
}

//Redirection
int executeRedir(char **command, char *inputName, char *outputName, int isAppend, int builtinNo){
    pid_t pid;
    if ((pid=fork())==-1) {
        fprintf(stderr,"Fork error.\n");
        return -1;
    } else if (pid == 0) {
        int fdIn;
        int fdOut;
        if(inputName[0]!='\0'){
            if((fdIn = open(inputName, O_RDONLY, 0777))==-1){
                fprintf(stderr,"%s\n","Cannot open input file.");
                exit(1);
            }
            dup2(fdIn, 0);
        }
        close(fdIn);
        if(outputName[0]!='\0'){
            if (isAppend){
                if((fdOut = open(outputName, O_WRONLY|O_CREAT|O_APPEND,S_IRWXU|S_IRWXG|S_IRWXO))==-1){
                    fprintf(stderr,"%s\n","Cannot process. Please check for invalid file name.");
                    exit(1);
                }
            } else {
                if((fdOut = open(outputName, O_WRONLY|O_CREAT|O_TRUNC,S_IRWXU|S_IRWXG|S_IRWXO))==-1){
                    fprintf(stderr,"%s\n","Cannot process. Please check for invalid file name.");
                    exit(1);
                }
            }
            dup2(fdOut, 1);
        }
        close(fdOut);
        if(builtinNo!=-1){
            if((*builtins[builtinNo])(command)!=0){
                fprintf(stderr,"%s %s\n","Failed to execute built-in command:", builtin_cmd[builtinNo]);
                exit(1);
            }
            exit(0);
        }

        if (execvp(command[0], command) != 0) {
            fprintf(stderr,"Cannot start the program.\n");
            exit(1);
		}
    } else {
        wait(NULL);
    }
    return 0;
}

//Pipe
int executePipe(char ** command1, char ** command2, int builtinNo1, int builtinNo2){
    int fd[2];
    pid_t pid;
    pipe(fd);
    if ((pid=fork())==0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        if(builtinNo1!=-1){
            if((*builtins[builtinNo1])(command1)!=0){
                fprintf(stderr,"%s %s\n","Failed to execute built-in command:", builtin_cmd[builtinNo1]);
                return -1;
            }
            exit(0);
        }
        if (execvp(command1[0], command1) != 0) {
            fprintf(stderr,"Cannot start the program.\n");
            exit(EXIT_FAILURE);
		}
    } else {
        if ((pid=fork())==0) {
            close(fd[1]);
            dup2(fd[0], STDIN_FILENO);
            if(builtinNo2!=-1){
                if((*builtins[builtinNo2])(command2)!=0){
                    fprintf(stderr,"%s %s\n","Failed to execute built-in command:", builtin_cmd[builtinNo2]);
                    return -1;
                }
                exit(0);
            }
            if (execvp(command2[0], command2) != 0) {
                fprintf(stderr,"Cannot start the second program.\n");
                exit(1);
            }
        }
        close(fd[0]);
    }
    wait(NULL);
    close(fd[0]);
    close(fd[1]);
    return 0;
}

//Checks for built-in commands
int checkBuiltins(char *args){
    if(strcmp(args,"quit")==0){
        exit(0);
    }
    int i = 0;
    for(;i<7;i++) {//number of builtin command with quit
        if(strcmp(args, builtin_cmd[i])==0){
            return i;
        }
    }
    return -1;
}

//Changes directory
int ex_cd(char **args){
    if (args[2]!=NULL){
        fprintf(stderr, "%s\n", "Too many argument.");
        return -1;
    }
    if (args[1] == NULL) {
        // to be filled
        char *home;
        if ((start = getenv("home")) == NULL)
            fprintf(stderr, "Cannot get $HOME");
        else {
            if (chdir(start) == -1){
                fprintf(stderr, "cd: %s\n", strerror(errno));
                return -1;
            }
        }
    } else {
        if (chdir(args[1]) == -1){
            fprintf(stderr, "cd: %s: %s\n", strerror(errno), args[1]);
            return -1;
        }
    }
    return 0;
}

//Clears screen
int ex_clr(char **args){
    if(args[1]!=NULL){
        fprintf(stderr, "%s\n", "Too many argument.");
        return -1;
    }
    system("clear");
    return 0;
}

//Reads directory
int ex_dir(char **args){
    if(args[2]!=NULL){
        fprintf(stderr, "%s\n", "Too many argument.");
        return -1;
    }
    DIR *dp;
    struct dirent *dir;
    char path[MAX_LENGTH];

		if (args[1] == NULL) {
			fprintf(stderr, "Error when executing dir: no path provided.\n");
			return -1;
		} else {
		    strcpy(path, args[1]);
			if ((dp = opendir(path)) == NULL) {
				fprintf(stderr, "Opendir %s error", path);
			}
			while ((dir = readdir(dp))) {
				printf("%s\n", dir -> d_name);
			}
			closedir(dp);
		}
    return 0;
}

//Prints out environment
int ex_environ(char **args){
    if(args[1]!=NULL){
        fprintf(stderr, "%s\n", "Too many argument.");
        return -1;
    }
    pid_t pid;
		if ((pid=fork())==-1) {
            fprintf(stderr, "Fork error: %s\n", strerror(errno));
            return -1;
		} else if (pid==0){
            system("printenv");
            return 0;
        } else {
            wait(NULL);
		}
    return 0;
}

//Echo strings
int ex_echo(char **args){
    char *echo_string = malloc(sizeof(char*) * MAX_LENGTH);
		int i = 1;
		while (args[i] != NULL) {
			strcat(echo_string,args[i]);
			strcat(echo_string," ");
			i++;
		}
		printf("%s\n",echo_string);
        strcpy(echo_string, "");
        free(echo_string);
    return 0;
}

//Opens readme and uses more to read
int ex_help(char **args){
    if(args[1]!=NULL){
        fprintf(stderr, "%s\n", "Too many argument.");
        return -1;
    }
    char *help = malloc(sizeof(char*) * MAX_LENGTH);
    strcat(help,"more ");
    strcat(help,originalDir);
    strcat(help,"/readme.txt");
    system(help);
    strcpy(help, "");
    free(help);
    return 0;
}

//Pauses the shell
int ex_pause(char **args){
    if(args[1]!=NULL){
        fprintf(stderr, "%s\n", "Too many argument.");
        return -1;
    }
    char c;
    printf("Operation paused. Press enter to continue.\n");
    while ((c = getchar()) != '\n') {
    }
    return 0;
}
