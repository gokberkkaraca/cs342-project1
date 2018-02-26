#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "sys/types.h"
#include "sys/wait.h"
#include "fcntl.h"

#define END_OF_DATA -1
#define READ_END 0
#define WRITE_END 1
#define MAX_INTEGERS 1000000
#define MAX_CHILDREN 50

/*************************
***QUEUE IMPLEMENTATION***
*************************/

struct Node {
  int data;
  struct Node *next;
};

struct Queue {
  struct Node *head, *tail;
};

struct Queue* createQueue() {
  struct Queue* temp = (struct Queue*) malloc(sizeof(struct Queue));
  temp->head = NULL;
  temp->tail = NULL;
  return temp;
}

void enqueue(struct Queue* queue, int data) {
  struct Node* node = (struct Node*) malloc(sizeof(struct Node));
  node->data = data;
  node->next = NULL;

  if (queue->tail == NULL) {
    queue->head = node;
    queue->tail = node;
  }
  else {
    queue->tail->next = node;
    queue->tail = node;
  }
}

int dequeue(struct Queue* queue) {
  if (queue->head == NULL) {
    return -1;
  }
  else {
    struct Node* node = queue->head;
    queue->head = queue->head->next;

    if (queue->head == NULL) {
      queue->tail = NULL;
    }
    int data = node->data;
    free(node);
    return data;
  }
}

int isEmpty(struct Queue* queue) {
  return queue->head == NULL;
}

/*************************
*****PRIME CALCULATOR*****
*************************/
void checkArguments(int argc, int numOfIntegers, int numOfChildren) {
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
}

int main(int argc, char **argv) {

  struct Queue* mainQueue = createQueue();

  int numOfIntegers = atoi(argv[1]);
  int numOfChildren = atoi(argv[2]);

  pid_t childProcesses[numOfChildren];
  pid_t mainPid = getpid();

  int childrenFDs[numOfChildren + 1][2];
  int printerPipe[2];

  // Check if the arguments are correct
  checkArguments(argc, numOfIntegers, numOfChildren);

  // Create pipes for children
  for (int i = 0; i < numOfChildren + 1; i++) {
    if (pipe(childrenFDs[i]) == -1) {
      fprintf(stderr, "Pipe opening failed.\n");
      exit(1);
    }
  }

  // Create pipe for printer
  if (pipe(printerPipe) == -1) {
    fprintf(stderr, "Pipe opening failed.\n");
    exit(1);
  }

  // Make the pipe between last child and main process non-blocking
  fcntl(childrenFDs[numOfChildren][READ_END], F_SETFL,O_NONBLOCK);

  // Insert all numbers to main QUEUE
  for (int i = 2; i <= numOfChildren; i++) {
    enqueue(mainQueue, i);
  }
  enqueue(mainQueue, -1);

  int i;
  for (i = 0; i < numOfChildren + 1; i++) {
    childProcesses[i] = fork();
    if (childProcesses[i] < 0) {
      fprintf(stderr, "Fork failed\n");
      exit(1);
    }
    else if (getpid() != mainPid) {
      int numberToRead;
      int numberToWrite;
      int primeNumber = END_OF_DATA;

      // Printer process
      if (i == numOfChildren) {
        while(1) {
          if (read(printerPipe[READ_END], &numberToRead, 1) > 0) {
            printf("Prime: %d\n", numberToRead);
          }
        }
      }
      // Child process
      else {
        while(1) {

          // Read the number from pipe
          if (read(childrenFDs[i][READ_END], &numberToRead, sizeof(numberToRead)) > 0) {
            printf("Read: %d\n", numberToRead);
          }

          // First number of the series is read, it is the prime number
          if (primeNumber == END_OF_DATA) {
            primeNumber = numberToRead;

            // Send prime number to printerPipe
            printf("Sending prime: %d\n", primeNumber);
            write(printerPipe[WRITE_END], &primeNumber, sizeof(primeNumber));
          }
          else {
            
          }

        }
      }
    }
  }

  // Parent process
  if (getpid() == mainPid) {
    printf("Inside main\n");
    //int numberToRead;
    int numberToWrite = 2;

    while (1) {
      if (!isEmpty(mainQueue)) {
        numberToWrite = dequeue(mainQueue);
        printf("Sending number: %d\n", numberToWrite);
        write(childrenFDs[0][WRITE_END], &numberToWrite, sizeof(numberToWrite));
      }
    }
  }

  return 0;
}
