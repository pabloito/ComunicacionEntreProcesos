#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#define MAXPATH_LEN 255


int executeCommand(char * command, char ** args);
int exists(const char *fname);

int main(int argc, char ** args)
{
  if(argc<2){
    printf("Please send at least one parameter\n");
    exit(-1);
  }
  char * command = args[1];

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
