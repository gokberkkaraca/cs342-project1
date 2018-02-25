#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "sys/types.h"
#include "sys/wait.h"

int main(int argc, char **argv) {

  int numOfIntegers = atoi(argv[1]);
  int numOfChildren = atoi(argv[2]);
  int integersArray[numOfIntegers - 1];
  pid_t childProcesses[numOfChildren];

  // Check if the number of arguments are correct
  if (argc != 3) {
    printf("Wrong number of arguments\n");
    printf("Usage: prime <numberofintegers> <numberofchildren>");
    exit(1);
  }

  // Create an array that includes number from 2 to N
  for (int i = 0; i < numOfIntegers - 1; i++) {
    integersArray[i] = i + 2;
  }

  for (int i = 0; i < numOfChildren; i++) {
    childProcesses[i] = fork();
    if (childProcesses[i] < 0) {
      fprintf(stderr, "Fork failed");
      exit(-1);
    }
    else if (childProcesses[i] == 0) {
      printf("I am a child process %d\n", childProcesses[i]);
    }
    else {
      wait(NULL);
      printf("Children complete, pid: %d\n", childProcesses[i]);
      exit(0);
    }
  }

  return 0;
}
