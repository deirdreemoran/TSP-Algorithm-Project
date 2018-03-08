/***********************************************************************
*
*  Author: Deirdre Moran
*  Program: smallsh.c
*  Date: 3/5/2018
*  Description: C shell
*
************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>

// max number of background processes
#define MAX_BG_PROCESSES 100
// max number of user entry arguments
#define MAX_ARGUMENTS 512
// max number of buffer characters
#define MAX_BUFFER 2048

// global variables for process handling and background modes
typedef enum { false, true } bool;
bool backgroundON = true;
bool bgProcess = false;
// background processes for clean up
pid_t bgPID[MAX_BG_PROCESSES];
// number of background processes
int bgSize = 0;
int childExitStatus;

/***********************************************************************
*  Function: 		catchSIGTSTP(int)
*  Description:  	Signal handler for SIGTSTP signal that controls if
*					background processes are allowed.  Prints
*					message to user depending on current mode.
*  Parameters:   	signal number
*  Pre-conditions:	None
*  Post-conditions:	background processes are allowed or disallowed
*  Return:			None
************************************************************************/
void catchSIGTSTP(int signo){
	char * message1 = "\nEntering foreground-only mode (& is now ignored)\n: ";
	char * message2 = "\nExiting foreground-only mode\n: ";
	// if background is in allow mode, change to disallow mode
	if (backgroundON == true){
		backgroundON = false;
		fflush(stdout);
		write(STDOUT_FILENO, message1, 52);
		fflush(stdout);
	}
	// if background is in disallow mode, change to allow mode
	else if(backgroundON == false){
		backgroundON = true;
		write(STDOUT_FILENO, message2, 32);
		fflush(stdout);
	}
}

/***********************************************************************
*  Function: 		bgFinishedProcesses
*  Description:  	Loops through background processes array and prints
*					exit value and pid of finished processes.
*  Parameters:   	None
*  Pre-conditions:	None
*  Post-conditions:	None
*  Return:			None
************************************************************************/
void bgFinishedProcesses(){
	int status;
	int arrayPID;
	int i;

	for (i = 0; i < bgSize; i++){
		// If there is a background process (that is not of value -1)
		if(bgPID[i] != -1){
			arrayPID = 0;
			//wait for background children to finish but don't block other processes
			arrayPID = waitpid(bgPID[i], &status, WNOHANG);
			if(arrayPID > 0){
				fflush(stdout);
				printf("background pid %d is done. ", arrayPID);
				fflush(stdout);
				if(WIFEXITED(status)){
					int exitStatus = WEXITSTATUS(status);
					fflush(stdout);
					printf("exit value %d\n", exitStatus);
					fflush(stdout);
				}
				else if(WIFSIGNALED(status) != 0){
					int termSignal = WTERMSIG(status);
					fflush(stdout);
					printf("terminated by signal %d \n", termSignal);
					fflush(stdout);
				}
				// clear the entry for completed child process
				bgPID[i] = -1;
			}
		}
	}
}

/***********************************************************************
*  Function: 		getUserCommand
*  Description:  	Gets user entry, ignores comments and blank lines.
*  Parameters:   	None
*  Pre-conditions:	None
*  Post-conditions:	None
*  Return:			lineEntered (user entry)
************************************************************************/
char * getUserCommand(){
	int numsChar;
	size_t buffersize = 0;
	//variable to hold user input
	char * lineEntered = NULL;
	while(1){//loop for getting user commands
		fflush(stdout);
		printf(": ");
		numsChar = getline(&lineEntered, &buffersize, stdin);
		if(numsChar == -1){
			clearerr(stdin);
		}
		//ignore comments and blank lines
		if(strncmp(lineEntered, "#", 1) == 0 || strlen(lineEntered) == 1){
			fflush(stdout);
			memset(lineEntered, '\0', buffersize);
		}
		//we have user input, break out of while loop
		else {
			break;
		}
	}
	return lineEntered;
}

/***********************************************************************
*  Function: 		cdCommand
*  Description:  	Opens directory, if none specified, go to home directory
*  Parameters:   	User entry tokenized array tokenArray, numArgs in tokenArray
*  Pre-conditions:	None
*  Post-conditions:	None
*  Return:			None
************************************************************************/
void cdCommand(char * myArray[], int numArgs){
	// If no directory is specified, go to home directory
	if(numArgs == 1){
		char * directory = getenv("HOME");
		chdir(directory);
	}
	// If directory is specified
	else{
		char currDir[2048];
		getcwd(currDir, sizeof(currDir));
		strcat(currDir, "/");
		strcat(currDir, myArray[1]);
		int sig = chdir(currDir);
		if(sig != 0){
			perror("No such directory");
		}
	}
}

/***********************************************************************
*  Function: 		exitCommand
*  Description:  	Exits program when user command is "exit".  Kills
*					any outstanding child processes, if any.
*  Parameters:   	None
*  Pre-conditions:	None
*  Post-conditions:	None
*  Return:			None
************************************************************************/
void exitCommand(){
	pid_t pid = fork();
	// if error forking child
	if(pid < 0){
		perror("error");
	}
	// if child, execute exit and kill all processes
	if(pid == 0){
		kill(getpid(), SIGKILL);
		fflush(stdout);
		if((execlp("kill", "kill", "-KILL", getpid(), getppid())) == -1){
			fprintf(stderr, "fail");
			exit(1);
		}
	}
	// if parent, wait for child then exit
	if (pid > 0){
		fflush(stdout);
		sleep(1);
		wait(&childExitStatus);
		fflush(stdout);
		exit(0);
	}
}

/***********************************************************************
*  Function: 		statusCommand
*  Description:  	Gives exit value or terminating signal of last command
*  Parameters:   	None
*  Pre-conditions:	None
*  Post-conditions:	None
*  Return:			None
************************************************************************/
void statusCommand(){
	// if there is an exit value
	if(WIFEXITED(childExitStatus)){
		int exitStatus = WEXITSTATUS(childExitStatus);
		fflush(stdout);
		printf("exit value %d\n", exitStatus);
		fflush(stdout);
	}
	// if process terminated by signal, show signal
	else if(WIFSIGNALED(childExitStatus) != 0){
		int termSignal = WTERMSIG(childExitStatus);
		fflush(stdout);
		printf("terminated by signal %d \n", termSignal);
		fflush(stdout);
	}
}


/***********************************************************************
*  Function: 		redirectCommand
*  Description:  	Executes commands with redirections
*  Parameters:   	filedescriptor array, redirection array, token array,
*					array for file, number of arguments in command
*  Pre-conditions:	None
*  Post-conditions:	None
*  Return:			None
************************************************************************/
void redirectCommand(int fd[], char * redirA[], char * tokenArray[], char * arrayA[], int numArgs){
	// for output redirection
	if(strncmp(redirA[0], ">", 1) == 0){
		//open file and set pipe to file descriptor 1
		fd[1] = open(tokenArray[numArgs -1], O_CREAT | O_TRUNC | O_WRONLY, 0664);
		if(fd[1] == -1){
			fflush(stdout);
			fprintf(stderr, "Cannot open %s for input", tokenArray[numArgs - 1]);
		}
			pid_t pid = fork();
			// if error forking child
			if(pid < 0){
				perror("error");
				exit(1);
			}
			// if child
			if (pid == 0){
				//make wherever stdin is pointing point to stdin
				dup2(0, STDIN_FILENO);
				//make wherever stdout is pointing now point to file where filedesriptor points to
				dup2(fd[1], STDOUT_FILENO);
				//close the file that was opened
				close(fd[0]);
				close(fd[1]);
				if((execvp(arrayA[0], arrayA)) == -1){
					fprintf(stderr, "fail");
					exit(1);
				}
			}
			// if parent
			if(pid > 0){
				fflush(stdout);
				sleep(1);
				wait(&childExitStatus);
			}
		}
		// for input redirection
		else if(strncmp(redirA[0], "<", 1) == 0){
		// open file for readonly
			fd[0] = open(tokenArray[numArgs - 1], O_RDONLY, 0664);
			if(fd[0] == -1){
				fprintf(stderr, "cannot open %s for input\n", tokenArray[numArgs - 1]);
			}
			else{
				pid_t pid = fork();
				// if problem forkinh
				if(pid < 0){
					printf("error");
					exit(1);
				}
				// if child
				if(pid == 0){
					dup2(fd[0], STDIN_FILENO);
					dup2(1, STDOUT_FILENO);
					close(fd[0]);
					int g;
					if((g = execlp(tokenArray[0], tokenArray[0], NULL)) == -1){
						fprintf(stderr, "Exit value 1");
					}
				}
				// if parent
				if(pid > 0){
					fflush(stdout);
					sleep(1);
					wait(&childExitStatus);
				}
			}
		}
}

/***********************************************************************
*  Function: 		multRedirectionCommand
*  Description:  	Executes commands with redirections
*  Parameters:   	filedescriptor array, redirection array, token array,
*					array for file, number of arguments in command
*  Pre-conditions:	None
*  Post-conditions:	None
*  Return:			None
************************************************************************/
void multRedirectionCommand(int fd[], char * redirA[], char * tokenArray[], int numArgs){
	// open file for reading
	fd[0] = open(tokenArray[numArgs - 3], O_RDONLY, 0664);
	// if error opening file
	if(fd[0] == -1) {
		printf("Cannot open %s for input", tokenArray[numArgs - 3]);
		childExitStatus = 1;
	}
	// open file for writing
	fd[1] = open(tokenArray[numArgs - 1], O_CREAT | O_TRUNC | O_WRONLY, 0664);
	// if error opening file
	if(fd[1] == -1){
		//printf("Cannot open %s for input", tokenArray[numArgs - 1]);}
		fprintf(stderr, "cannot open %s for input", tokenArray[numArgs - 1]);
	}
	pid_t pid = fork();
	if(pid < 0){
		printf("error");
		exit(1);
	}
	// if child process
	if(pid == 0){
		dup2(fd[0], STDIN_FILENO);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		if((execlp("wc", "wc", NULL)) == -1){
			fprintf(stderr, "fail");
			exit(1);
		}
	}
	//if parent process
	if(pid > 0){
		fflush(stdout);
		sleep(1);
		wait(&childExitStatus);
	}
}

/***********************************************************************
*  Function: 		otherCommands
*  Description:  	For all other commands
*  Parameters:   	sigaction structs for SIGINT, SIGTSTP, signal ignore
*					token array of commands, sigprocmask
*  Pre-conditions:	None
*  Post-conditions:	None
*  Return:			None
************************************************************************/
void otherCommands(struct sigaction SIGINT_action, struct sigaction ignore_action, struct sigaction SIGTSTP_action, char *tokenArray[], sigset_t x){
	pid_t pid = fork();
	//if error forking
	if(pid < 0){
		printf("error");
		exit(1);
	}
	//child process
	if (pid == 0){
		int g;
		fflush(stdout);
		// if this is a background process
		if(bgProcess == true){
			SIGTSTP_action.sa_handler = SIG_IGN;
			sigaction(SIGTSTP, &SIGTSTP_action, NULL);
			fflush(stdout);
			if((g = execvp(tokenArray[0], tokenArray)) == -1){
				fflush(stdout);
				printf("%s: No such file or directory\n", tokenArray[0]);
				exit(1);
				fflush(stdout);
			}
		}
		// if foreground process
		else{
			SIGINT_action.sa_handler = SIG_DFL;
			sigaction(SIGINT, &SIGINT_action, NULL);
			sigprocmask(SIG_BLOCK, &x, NULL);
			if((g = execvp(tokenArray[0], tokenArray)) == -1){
				fflush(stdout);
				printf("%s: No such file or directory\n", tokenArray[0]);
				exit(1);
			}
		}
	}
	//if parent process
	if(pid > 0){
		fflush(stdout);
		// if foreground process
		if(bgProcess == false ){
			//sigprocmask(SIG_BLOCK, &x, NULL);
			waitpid(pid, &childExitStatus, NULL);
			sigprocmask(SIG_UNBLOCK, &x, NULL);
			fflush(stdout);
			if(WIFSIGNALED(childExitStatus) != 0){
				int termSignal = WTERMSIG(childExitStatus);
				fflush(stdout);
				printf("terminated by signal %d\n", termSignal);
				fflush(stdout);
			}
		}
		// if background process
		else if(bgProcess == true){
			fflush(stdout);
			printf("background pid is %d\n", pid);
			bgPID[bgSize] = pid;
			bgSize++;
		}
	}
}

/************************************************************************
*************************************************************************
*************************************************************************
*************************************************************************
***********												      ***********
****							MAIN() 			                      ***
***********												      ***********
*************************************************************************
*************************************************************************
*************************************************************************
************************************************************************/
void main(){
	// declare variables
	// file descriptors
	int fd[2];
	// hilds return value for error reporting getline
	int numsChar;
	// size of entry
	size_t buffersize = 0;
	//variable to hold getline user input
	char * lineEntered = NULL;
	int i;
	// child exit status
	int status;
		//holds char version of pid
	char piddy[10] = "         ";
	snprintf(piddy, 10, "%d", (int)getpid());
	// holds tokenized user input
	char * tokenArray[MAX_ARGUMENTS];
	char * arrayA[MAX_ARGUMENTS];

	// signal handling definitions
	struct sigaction SIGINT_action = {0};
	struct sigaction ignore_action = {0};
	struct sigaction SIGTSTP_action = {0};

	// SIGINT handler for parent process set to ignore SIGINT signals
	SIGINT_action.sa_handler = SIG_IGN;
	sigfillset(&SIGINT_action.sa_mask);
	SIGINT_action.sa_flags = 0;

	// SIGTSTP handler for parent process set to catch TSTP signals
	SIGTSTP_action.sa_handler = catchSIGTSTP;
	sigfillset(&SIGTSTP_action.sa_mask);
	SIGTSTP_action.sa_flags = SA_RESTART;

	// Ignore handler
	ignore_action.sa_handler = SIG_IGN;

	// Set sigactions for parent process
	sigaction(SIGINT, &SIGINT_action, NULL);
	sigaction(SIGTSTP, &SIGTSTP_action, NULL);

	// Add SIGTSTP signal to sigset for later use in temporarily blocking this signal
	sigset_t x;
	sigemptyset(&x);
	sigaddset(&x, SIGTSTP);

	// Main while loop for shell program
	while(1){
		// Set/set action handler for parent process
		SIGINT_action.sa_handler = SIG_IGN;
		sigfillset(&SIGINT_action.sa_mask);
		SIGINT_action.sa_flags = 0;
		// Rest background process flag to false
		bgProcess = false;

		// Handle finished background processes
		bgFinishedProcesses();
		// Get user command
		lineEntered = getUserCommand();

		// variables for formatting process ID $$
		char spc[2] = " ";
		char * token;
		char * tokenCopy;
		char tokeee[20];
		memset(tokeee, 0, sizeof(tokeee));
		int numArgs = 0;
		//	initialize tokenArray to hold commands after user input is parsed
		memset(tokenArray, '\0', sizeof(tokenArray));
		token = strtok(lineEntered, spc);

		//tokenize line by spaces
		while(token != NULL){
			tokenCopy = token;
			char * g;
			fflush(stdout);
			// search token for process id $$
			g = strstr(token, "$$");
			// if process id in token, reformat it to expand number of current process id
			if(g){
				g = piddy;
				char * newline = strchr(tokenCopy, '$');

				if (newline){
					*newline = 0;
				}
				// concatenate beginning of token (without process id) to empty string
				strcat(tokeee, tokenCopy);
				// concatenate expanded process id onto original beginning portion
				strcat(tokeee, piddy);
				g = piddy;
				// store in token array, increase args, move to next token
				tokenArray[numArgs] = tokeee;
				numArgs++;
				token = strtok(NULL, spc);
			}
			// for tokens not containing "$$" process id
			else{
				tokenArray[numArgs] = token;
				numArgs++;
				token = strtok(NULL, spc);
			}
		}

		//remove newlines, if any
		int j;
		for (j = 0; j < numArgs; j++){
			char * newline = strchr(tokenArray[j], '\n');
			if (newline){
				*newline = 0;
			}
		}
		//holds arguments separate for redirection commands
		char * arrayA[MAX_ARGUMENTS];
		memset(arrayA, '\0', MAX_ARGUMENTS);
		char * redirA[MAX_ARGUMENTS];
		// counters for number of redirection commands
		int tempy = 0;
		j = 0;
		int arAid = 0;

		// if the process is to be run in background, change bgProcess to true for flagging later
		if(strcmp(tokenArray[numArgs - 1], "&") == 0){
			tokenArray[numArgs - 1] = '\0';
			numArgs--;
			if(backgroundON == true){
				bgProcess = true;
			}
			else{
				bgProcess = false;
			}
		}

		// loop through all arguments
		for (i = 0; i < numArgs; i++){
			// if redirection operator for output
			if(strncmp(tokenArray[i], ">", 1) == 0){
				tempy++;
				redirA[j] = tokenArray[i];
				j++;
			}
			// if redirection for input
			else if(strncmp(tokenArray[i], "<", 1) == 0){
				tempy++;
				redirA[j] = tokenArray[i];
				j++;
			}
			// if no redirection
			else if(tempy == 0 && tokenArray[i] != NULL){
				arrayA[arAid] = tokenArray[i];
				arAid++;
				// if token is process id, expand
				if(strcmp(tokenArray[i], "$$") == 0){
					strcpy(tokenArray[i], piddy);
					strcpy(arrayA[arAid - 1], piddy);
				}
			}
		}

		fflush(stdout);
		//change directory command
		if(strncmp(tokenArray[0], "cd", 2) == 0){
			cdCommand(tokenArray, numArgs);
		}
		// if user enters exit command, exit program
		else if(strcmp(arrayA[0], "exit") == 0){
			exitCommand();
		}
		//if user command enters status, return exit value of last command
		else if(strcmp(arrayA[0], "status") == 0){
			statusCommand();
		}
		//if there is one redirection operator
		else if(tempy == 1 ){
			redirectCommand(fd, redirA, tokenArray, arrayA, numArgs);
		}
		//if there is more than one redirection
		else if (tempy > 1){
			multRedirectionCommand(fd, redirA, tokenArray, numArgs);
		}
		//if there are no redirection operators and command is not cd, status, or exit
		else if (tempy == 0){
			otherCommands(SIGINT_action, ignore_action, SIGTSTP_action, tokenArray, x);
		}

		//free and clear memory for reuse
		memset(lineEntered, '\0', buffersize);
		free(lineEntered);
		lineEntered = NULL;
		fflush(stdout);
	}//end program while loop
}// end main
