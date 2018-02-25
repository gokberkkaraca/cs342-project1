#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "sys/types.h"
#include "sys/wait.h"

#define END_OF_DATA -1
#define MAX_INTEGERS 1000000
#define MAX_CHILDREN 50

int main(int argc, char **argv) {

  int numOfIntegers = atoi(argv[1]);
  int numOfChildren = atoi(argv[2]);

  // Check if the arguments are correct
  if (argc != 3) {
    printf("Wrong number of arguments\n");
    printf("Usage: prime <numberofintegers> <numberofchildren>");
    exit(1);
  }
  else if (numOfIntegers > MAX_INTEGERS) {
    printf("Invalid number of integers\n");
    printf("Maximum 1000000 integers are allowed");
    exit(1);
  }
  else if(numOfChildren > MAX_CHILDREN) {
    printf("Invalid number of children\n");
    printf("Maximum 50 children are allowed");
    exit(1);
  }

  pid_t childProcesses[numOfChildren];
  //pid_t printerProcess;

  for (int i = 0; i < numOfChildren; i++) {
    childProcesses[i] = fork();
    if (childProcesses[i] < 0) {
      fprintf(stderr, "Fork failed");
      exit(-1);
    }
    else if (childProcesses[i] == 0) { // Children runs here
      printf("I am a child process %d\n", childProcesses[i]);
    }
    else { // Parent runs here
      wait(NULL);
      printf("Children complete, pid: %d\n", childProcesses[i]);
      exit(0);
    }
  }

  return 0;
}
