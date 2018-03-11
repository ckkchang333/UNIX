#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 4096

char** read_input(size_t size, int exit_value, char *prompt);

int fork_execute(char **argv);

void interrupt_handler(int signalNum);

void redirect(char **argv);

void set_starting_prompt(char** prompt);

void remove_redir(char** argv, int *index);

void print_argv(char **argv) {
 int i = 0;
 printf("Printing argv\n");
 while(argv[i] != NULL) {
  printf("%d: %s\n", i, argv[i]);
  ++i;
 }
 printf("Finished printing\n");
}


int main() {
 int exit_value = 0;
 char * prompt = (char *) malloc(sizeof(char) * 256);
 
 if(prompt == NULL) {
  perror("Initial malloc for prompt failed");
  exit(EXIT_FAILURE);
 }
 set_starting_prompt(&prompt);
 while (1) {
   // Input is read from the command line and tokenized
   char ** argv = (char **) read_input(BUFFER_SIZE, exit_value, prompt);


   // Implementaiton of "cd" and "exit"
   if(strcmp(argv[0], "cd") == 0) {
    chdir(argv[1]);
   }
   else if(strcmp(argv[0], "exit") == 0) {
    exit(EXIT_SUCCESS);
   }

   // Check for new prompt string
   else if(strstr(argv[0], "PS1=") != NULL && argv[0][strlen(argv[0]) - 1] == '"') {
    memset(prompt, 0, sizeof(char) * 256);
    strncpy(prompt, argv[0] + 5, strlen(argv[0]) - 6);
   }
   else if(strlen(argv[0]) == 0) {
    continue;
   }
   else {
    // Store the exit value or signal number + 128 
    // So that it can be printed to stdout on command
    exit_value = fork_execute(argv);
   }
   free(argv);
 }
 free(prompt);
}


// Tokenizes the command line
char** read_input(size_t size, int exit_value, char *prompt) {
 printf("%s ", prompt);
 char *input = malloc(sizeof(char) * BUFFER_SIZE);
 if(input == NULL) {
  perror("Malloc for input in read_input failed");
  exit(EXIT_FAILURE);
 }

 // Read from stdin
 getline(&input, &size, stdin);
 int i = 0;
 char** argv = malloc(sizeof(char) * BUFFER_SIZE); 
 if(argv == NULL) {
  perror("Malloc for argv in read_input failed");
  exit(EXIT_FAILURE);
 }
 
 // Tokenize the input
 char *token = strtok(input, " ");
 while(token != NULL) {
  if(token[strlen(token) - 1] == '\n') {
   token[strlen(token) - 1] = '\0';
  }
  // Replace "$?" with the latest exit value or signal number + 128
  if(strcmp(token, "$?") == 0) {
   char *exit_string = malloc(sizeof(char) * 3);
   sprintf(exit_string, "%d", exit_value);
   token = exit_string;
  }
  argv[i++] = token;
  token = strtok(NULL, " ");
 }

 return(argv);
}

// Creates a child process to execute the command from the command line
int fork_execute(char **argv) {

 // Child process
 if(fork() == 0) {
  redirect(argv);
  int exit_value = execvp(argv[0], argv);
  if (exit_value == -1) {
   perror("Command execution in fork_execute faield");
   exit(EXIT_FAILURE);
  }
  exit(EXIT_SUCCESS);
 }

 // 
 else {
  // The parent process (shell) should not be affected by SIGINT and SIGQUIT
  signal(SIGINT, interrupt_handler);
  signal(SIGQUIT, interrupt_handler);


  int status = 0;
  if(wait(&status) < 0) {
   perror("Parent process wait failed");
   exit(EXIT_FAILURE);
  }
  
  // First check if the child proccess ended with a signal
  if(WIFSIGNALED(status)) {
   return WTERMSIG(status) + 128;
  }
  // Then check if the child process ended on its own
  else if(WIFEXITED(status)) {
   return WEXITSTATUS(status);
  }
  else {
   exit(EXIT_FAILURE);
  }
 }
}



void interrupt_handler(int signalNum) {
}

// Sets the starting prompt depending if the env PS1 
void set_starting_prompt(char** prompt) {
 char *env_PS1;

 // Checks for a user-set PS1 environment variable
 if((env_PS1 = getenv("PS1")) != NULL) {
  strncpy(*prompt, env_PS1, strlen(env_PS1) + 1);
 }

 // Default prompt
 else {
  strncpy(*prompt, ">", 2);
 }
}



// Handles I/O Redirection
void redirect(char **argv) {
 int i = 0;

 while(argv[i] != NULL) {
  ++i;
 }
 --i;

 while(i >= 0) {

  // Stdout redirection
  if(strcmp(argv[i], ">") == 0) {
   int fd = open(argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0664);
   if(fd < 0) {
    perror("Open for stdout redirection in redirct failed");
    exit(EXIT_FAILURE);
   }
   if(dup2(fd, 1) < 0) {
    perror("Dup for stdout redirection in redirect failed");
    exit(EXIT_FAILURE);
   }
   close(fd);
   if (memset(argv[i], 0, strlen(argv[i]) + 1) == NULL) {
    perror("Memset A for stdout redirection failed");
    exit(EXIT_FAILURE);
   }
   if (memset(argv[i + 1], 0, strlen(argv[i + 1]) + 1) == NULL) {
    perror("Memset B for stdout redirection failed");
    exit(EXIT_FAILURE);
   }
   remove_redir(argv, &i);
  }

  // Stdout append redirection  
  else if(strcmp(argv[i], ">>") == 0) {
   int fd = open(argv[i + 1], O_WRONLY | O_APPEND | O_CREAT, 0664);
   if(fd < 0) {
    perror("Open for stdout append redirection in redirct failed");
    exit(EXIT_FAILURE);
   }
   if(dup2(fd, 1) < 0) {
    perror("Dup for stdout append redirection failed");
    exit(EXIT_FAILURE);
   }
   close(fd);
   if (memset(argv[i], 0, strlen(argv[i]) + 1) == NULL) {
    perror("Memset A for stdout append redirection failed");
    exit(EXIT_FAILURE);
   }
   if (memset(argv[i + 1], 0, strlen(argv[i + 1]) + 1) == NULL) {
    perror("Memset B for stdout append redirection failed");
    exit(EXIT_FAILURE);
   }
   remove_redir(argv, &i);
  }

  // Stdin redirection
  else if(strcmp(argv[i], "<") == 0) {
   int fd = open(argv[i + 1], O_RDONLY);
   if(fd < 0) {
    perror("Open for stdin redirection in redirct failed");
   }
   if(dup2(fd, 0) < 0) {
    perror("Dup for stdin redirect failed");
   }
   close(fd);
   if (memset(argv[i], 0, strlen(argv[i]) + 1) == NULL) {
    perror("Memset A for stdin redirection failed");
   }
   if (memset(argv[i + 1], 0, strlen(argv[i + 1]) + 1) == NULL) {
    perror("Memset B for stdin redirection failed");
   }
   remove_redir(argv, &i);
  }

  // Stderr redirection
  else if(strcmp(argv[i], "2>") == 0) {
   int fd = open(argv[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0664);
   if(fd < 0) {
    perror("Open for stderr redirection in redirct failed");
    exit(EXIT_FAILURE);
   }
   if(dup2(fd, 2) < 0) {
    perror("Dup for stderr redirection failed");
    exit(EXIT_FAILURE);
   }
   close(fd);
   if (memset(argv[i], 0, strlen(argv[i]) + 1) == NULL) {
    perror("Memset A for stderr redirection failed");
    exit(EXIT_FAILURE);
   }
   argv[i] = NULL;
  }
  --i;
 }
}

// Removes the redirection portion from the command line
// after redirection is completed
void remove_redir(char** argv, int *index) {
 int old = *index; 
 while(argv[(*index) + 2] != NULL) {
  argv[*index] = argv[(*index) + 2];
  ++(*index);
 }
 argv[*index] = NULL;
 argv[(*index) + 1] = NULL;
 (*index) = old;
}
