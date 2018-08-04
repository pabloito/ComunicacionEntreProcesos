#include "runCommandLine.h"
//https://stackoverflow.com/questions/28507950/calling-ls-with-execv
//https://stackoverflow.com/questions/230062/whats-the-best-way-to-check-if-a-file-exists-in-c-cross-platform
//https://jineshkj.wordpress.com/2006/12/22/how-to-capture-stdin-stdout-and-stderr-of-child-program/

int pipes[NUM_PIPES][2];

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

      char parity = parityByte(input, size);
      char * bytes = charToHex(parity);

      free(input);

      fprintf(stderr, "in parity: %s\n",bytes);

      close(PRODUCER_WRITE_FD);

    }
  }
  else{ //CONSUMER PROCESS
    close(PRODUCER_WRITE_FD);
    close(FILTER_READ_FD);
    close(FILTER_WRITE_FD);

    char buffer[10]={0};
    int readSize =0, size=0, allocsize=INITIAL_INPUT_SIZE;
    char * string = malloc(allocsize);

    while((readSize = read(CONSUMER_READ_FD,buffer,10))!=0){
      if(size==allocsize){
        allocsize+=INITIAL_INPUT_SIZE;
        string = realloc(string, allocsize);
      }
      size=+readSize;

      strcat(string,buffer);
      resetBuffer(buffer,10);
    }
    char parity = parityByte(string,size);
    char * bytes = charToHex(parity);


    fprintf(stderr, "in parity: %s\n",bytes);

    printf("%s",string);
    putchar('\n');

    free(string);
    close(CONSUMER_READ_FD);
  }
}


int executeCommand(char * command, char ** args)
{
  if(command==NULL){
    return -1;
  }
  char arg[MAXPATH_LEN]={0};
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
  char * buffer = malloc(INITIAL_INPUT_SIZE);
  int size=INITIAL_INPUT_SIZE;
  while((c=getchar())!=EOF){
    if(counter==size){
      size+=INITIAL_INPUT_SIZE;
      buffer = realloc(buffer, size);
    }
    *(buffer+counter)=c;
    counter++;
  }
  *bufferPosition=buffer;
  return counter;
}

void resetBuffer(char * buffer, int size)
{
  for(int i=0; i<size; i++){
    *(buffer+i)=0;
  }
}

char parityByte(char * string, int size)
{
  if(size==1){
    return string[0];
  }
  if(size==2){
    return string[0]^string[1];
  }
  int size1=size/2;
  return (parityByte(string,size1)^parityByte(string+size1,size-size1));
}

char * charToHex(char ch)
{
  char * ret=malloc(5);

  char coefficientNumber = ch/16;
  char surplusNumber = ch%16;

  char coefficient=hxNumberToChar(coefficientNumber);
  char surplus=hxNumberToChar(surplusNumber);

  ret[0]='0';
  ret[1]='x';
  ret[2]=coefficient;
  ret[3]=surplus;
  ret[4]=0;

  return ret;
}

char hxNumberToChar(char number)
{
  if(isDigit(number)){
    return number+48;
  }
  if(number==10) return 'A';
  if(number==11) return 'B';
  if(number==12) return 'C';
  if(number==14) return 'D';
  if(number==13) return 'E';
  if(number==15) return 'F';
  return 0;
}

int isDigit(char c)
{
  if(c==0||c==1 || c==2 || c==3 || c==4 || c==5 || c==6 || c==7 || c==8 || c==9){
    return 1;
  }
  return 0;
}
