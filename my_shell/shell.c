#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define BUFF_SIZE 1024 // size of input buffer
#define LSH_TOK_BUFSIZE 64 // size of argument array
#define LSH_TOK_DELIM " \t\r\n\a" // delimiters

int lsh_execute_command(char ** args);
int lsh_launch(char **args);
char* lsh_read_command();
char** lsh_split_command();
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

/*
  List of builtin commands, followed by their corresponding functions.
 */
char *builtin_str[] = {
  "cd",
  "help",
  "exit"
};

// create an array of function pointers, and assign it the addresses of the functions.
int (*builtin_func[]) (char **) = {
  &lsh_cd,
  &lsh_help,
  &lsh_exit
};


int lsh_num_builtins() {
  return sizeof(builtin_str) / sizeof(char *);
}

/*
  Builtin function implementations.
*/

int lsh_cd(char ** args){
	if (args[1] == NULL) {
   		 fprintf(stderr, "lsh: expected argument to \"cd\"\n");
  	} else {
		if (chdir(args[1]) != 0) { // invoke chdir syscall.
			perror("lsh");
		}
	}

	return 1;
}

int lsh_help(char ** args){
	int i;
	printf("Sonal's LSH\n");
  	printf("Type program names and arguments, and hit enter.\n");
  	printf("The following are built in:\n");

  	for (i = 0; i < lsh_num_builtins(); i++) {
    		printf("  %s\n", builtin_str[i]);
  	}

  	printf("Use the man command for information on other programs.\n");
  	return 1;
}

int lsh_exit(char **args)
{
  return 0;
}

void lsh_loop(){

	char *command;
	char **args;
	int status;

	do {
		printf(">");
		command = lsh_read_command();
		args = lsh_split_command(command);
		status = lsh_execute_command(args);
	} while(status);

	free(command);
	free(args);
}

char* lsh_read_command(){
	int buff_size = BUFF_SIZE;
	int position = 0;
	char* buffer = malloc(sizeof(char) * buff_size);
	int c;

	if(!buffer){
		fprintf(stderr, "Buffer allocation failure\n");
		exit(EXIT_FAILURE);
	}

	while(1){
		// read one char at a time
		c = getchar();

		if (c == EOF || c == '\n'){
			// replace with null character
			buffer[position] = '\0';
			return buffer;
		}
		else{
			buffer[position] = c;
		}
		position++;

		// if buffer limit is exceeded, increase size
		if (position >= buff_size){
			buff_size += BUFF_SIZE;
			buffer = realloc(buffer, buff_size);
			if(!buffer){
         		       fprintf(stderr, "Buffer allocation failure\n");
                		exit(EXIT_FAILURE);
        		}

		}
	}
}

char** lsh_split_command(char* line){
	int bufsize = LSH_TOK_BUFSIZE;
	int position = 0;
	char** tokens = malloc(sizeof(char*) * bufsize);
	char* token;

	if (!tokens){
		fprintf(stderr, "Buffer allocation failure\n");
		exit(EXIT_FAILURE);
	}

	// function in C used for breaking a string into smaller parts based on a set of delimiter characters.	
	token = strtok(line, LSH_TOK_DELIM); // gets the first token from the string.
	while (token != NULL){
		tokens[position] = token;
		position++;

		if (position >= bufsize){
			bufsize += LSH_TOK_BUFSIZE;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens){
                		fprintf(stderr, "Buffer allocation failure\n");
        		        exit(EXIT_FAILURE);
        		}
		}

		token = strtok(NULL, LSH_TOK_DELIM); // gets the next token, continuing from where it left off.
	}
	tokens[position] = NULL;
	return tokens;
}

int lsh_launch(char **args){
	pid_t pid;
	int status;

	pid = fork();

	if (pid == 0){
		// child process
		// replace the child process with the command (execvp()).
		if(execvp(args[0], args) == -1){ // execvp runs the command you give it by 
						 // replacing the current process with that command’s program.
			perror("lsh");
		}
		exit(EXIT_FAILURE);
	}
	else if (pid < 0){
		// Error forking
    		perror("lsh");
	}
	else { // parent process
	       do {
		       // parent waits for the child to finish
		       waitpid(pid, &status, WUNTRACED);
	       } while(!WIFEXITED(status) && !WIFSIGNALED(status)); // while child has not exited normally
	}
	return 1;
}

int lsh_execute_command(char ** args){
	int i;

	if (args[0] == NULL) {
   		 // An empty command was entered.
    		return 1;
  	}

	for (i = 0; i < lsh_num_builtins(); i++) {
  		if (strcmp(args[0], builtin_str[i]) == 0) {
      			return (*builtin_func[i])(args);
    		}
  	}

 	 return lsh_launch(args);
}

int main(int argc, char **argv){
	
	lsh_loop();

	return EXIT_SUCCESS;
}
