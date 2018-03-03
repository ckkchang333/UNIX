#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define buffer_size 4096

char** read_input(size_t size);

int fork_execute(char **argv);

void change_prompt(char **argv, char** prompt);


void print_argv_letters(char ** argv) {
 int i = 0;
 while(argv[i] != NULL) {
  int j = 0;
  while(argv[i][j] != NULL) {
   printf("%d: %c\n", j, argv[i][j]);
   ++j;
  }
 ++i;
 }
 printf("\n");
}

void print_argv(char ** argv) {
 int i = 0;
 while(argv[i] != NULL) {
  printf("%d: %s\n", i, argv[i]);
 ++i;
 }
 printf("\n");
}



int main() {
 //char prompt[256] = ">";
 char * prompt = (char *) malloc(sizeof(char) * 256);
 //memset(prompt, 0, sizeof(char) * 256);
 while (1) {
   char ** argv = (char **) read_input(buffer_size);
   //memset(prompt, 0, sizeof(char) * 256);
   //print_argv(argv);
   //printf("%s: %x\n", "printing first argument", (int) strtol(argv[0], NULL, 16));
   if(strcmp(argv[0], "cd") == 0) {
    printf("cd or exit\n");
    chdir(argv[1]);
   }
   else if(strcmp(argv[0], "exit") == 0) {
    return EXIT_SUCCESS;
   }
   else if(strstr(argv[0], "PS1=") != NULL && argv[0][strlen(argv[0]) - 1] == '"') {
    printf("%s\n", "In change prompt section");
    printf("%s: %s\n", "prompt", prompt);
    memset(prompt, 0, sizeof(char) * 256);
    
    printf("%s: %s\n", "argv[0]", argv[0]);
    printf("%li\n", strlen(argv[0]) - 6);
    strncpy(prompt, argv[0] + 5, strlen(argv[0]) - 6);
    printf("%s\n", "Changed prompt");
    printf("%s: %s\n", "prompt", prompt);
   }
   else if(strlen(argv[0]) == 0) {
    continue;
   }
   else {
    fork_execute(argv);
   }
   //printf("%s: %li\n", "Size of argv's first argument", strlen(argv[0]));
   //printf("\n\n\n");
   
   free(argv);
 }
 free(prompt);
}

char** read_input(size_t size) {
 printf("%c ", '>');
 char *input = malloc(sizeof(char) * buffer_size);
 if(input == NULL) {
  perror("Malloc for input failed");
 }
 getline(&input, &size, stdin);
 int i = 0;
 char** argv = malloc(sizeof(char) * buffer_size); 
 if(argv == NULL) {
  perror("Malloc for argv failed");
 }
 char *token = strtok(input, " ");
 while(token != NULL) {
  if(token[strlen(token) - 1] == '\n') {
   token[strlen(token) - 1] = '\0';
  }
  //token[strlen(token) - 1] = '\0';
  argv[i++] = token;
  token = strtok(NULL, " ");
 }

 return(argv);
}

int fork_execute(char **argv) {
 if(fork() == 0) {
    if (execvp(argv[0], argv) == -1) {
     perror("command failed to execute");
    }
    return EXIT_FAILURE;
   }
   else {
    int status;
    wait(&status);
   }
 return EXIT_SUCCESS;
}

void change_prompt(char** argv, char** prompt) {
 //if() {}
}

