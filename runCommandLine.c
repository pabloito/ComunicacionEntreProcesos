#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MAXPATH_LEN 255
#define MAX_INPUT_SIZE  100


//https://stackoverflow.com/questions/28507950/calling-ls-with-execv
//https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c-cross-platform
//https://jineshkj.wordpress.com/2006/12/22/how-to-capture-stdin-stdout-and-stderr-of-child-program/


int executeCommand(char * command, char ** args);
int exists(const char *fname);
int fetchInputFromStdin(char ** bufferPosition);


#define NUM_PIPES          2

#define FILTER_READ_PIPE   0
#define FILTER_WRITE_PIPE  1

int pipes[NUM_PIPES][2];

/* always in a pipe[], pipe[0] is for read and
   pipe[1] is for write */
#define READ_FD  0
#define WRITE_FD 1

#define CONSUMER_READ_FD  ( pipes[FILTER_WRITE_PIPE][READ_FD]   )
#define PRODUCER_WRITE_FD ( pipes[FILTER_READ_PIPE][WRITE_FD] )

#define FILTER_READ_FD   ( pipes[FILTER_READ_PIPE][READ_FD]  )
#define FILTER_WRITE_FD  ( pipes[FILTER_WRITE_PIPE][WRITE_FD]  )



int main(int argc, char ** args)
{
  if(argc<2){
    printf("Please send at least one parameter\n");
    exit(-1);
  }
  char * command = args[1];

  //initialize pipes
  if(pipe(pipes[FILTER_READ_PIPE])==-1){
    perror("pipe");
    exit(-1);
  }
  if(pipe(pipes[FILTER_WRITE_PIPE])==-1){
    perror("pipe");
    exit(-1);
  }

  pid_t pid;

  pid = fork();

  if(pid==0){ //CHILD PROCESS

    pid=fork();

    if(pid==0){ //FILTER PROCESS
      close(CONSUMER_READ_FD);
      close(PRODUCER_WRITE_FD);

      dup2(FILTER_READ_FD, STDIN_FILENO);
      dup2(FILTER_WRITE_FD, STDOUT_FILENO);

      close(FILTER_READ_FD);
      close(FILTER_WRITE_FD);

      int result = executeCommand(command,args+1);

      switch(result){
          case 0:
          break;
          case 1:
            printf("Command does not exist\n");
          break;
          case -1:
            printf("Received a null command\n");
          break;
        }
        return 1;

    }
    else{ //PRODUCER PROCESS
      close(FILTER_READ_FD);
      close(FILTER_WRITE_FD);
      close(CONSUMER_READ_FD);

      char * input;
      int size = fetchInputFromStdin(&input);

      write(PRODUCER_WRITE_FD, input, size);

      close(PRODUCER_WRITE_FD);

    }
  }
  else{ //CONSUMER PROCESS
    close(PRODUCER_WRITE_FD);
    close(FILTER_READ_FD);
    close(FILTER_WRITE_FD);
    char finished=0;
    char buffer[10]={0};
    while(!finished){
      finished=!read(CONSUMER_READ_FD,buffer,10);
      printf("%s",buffer);
    }
    putchar('\n');
    close(CONSUMER_READ_FD);
  }
}


int executeCommand(char * command, char ** args)
{
  if(command==NULL){
    return -1;
  }
  char * arg = malloc(MAXPATH_LEN);
  strcpy(arg,"/bin/");
  strcat(arg,command);

  if(exists(arg)){
    execv(arg, args);
    return 0;
  }
  return 1;
}

int exists(const char *fname)
{
    FILE *file;
    if ((file = fopen(fname, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

int fetchInputFromStdin(char ** bufferPosition)
{
  char c;
  int counter=0;
  char * buffer = malloc(MAX_INPUT_SIZE);
  while((c=getchar())!=EOF){
    *(buffer+counter)=c;
    counter++;
  }
  *bufferPosition=buffer;
  return counter;
}
