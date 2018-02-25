#include "stdio.h"
#include "stdlib.h"

int main(int args, char **argv) {

  int numOfIntegers = atoi(argv[1]);
  int numOfChildren = atoi(argv[2]);

  int integersArray[numOfIntegers - 1];
  for (int i = 0; i < numOfIntegers - 1; i++) {
    integersArray[i] = i + 2;
  }

  for (int i = 0; i < numOfChildren; i++) {

  }

  return 0;
}
