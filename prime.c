#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "fcntl.h"
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define END_OF_DATA -1
#define READ_END 0
#define WRITE_END 1
#define MAX_INTEGERS 1000000
#define MIN_INTEGERS 1000
#define MAX_CHILDREN 50
#define MIN_CHILDREN 1
#define KILL_SIGNAL -3

/*************************
***QUEUE IMPLEMENTATION***
*************************/

// This queue implementation was inspired from
// https://www.geeksforgeeks.org/queue-set-2-linked-list-implementation/

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
    printf("Usage: prime <numberofintegers> <numberofchildren>\n");
    exit(1);
  }
  if (numOfIntegers > MAX_INTEGERS) {
    printf("Invalid number of integers\n");
    printf("Maximum 1000000 integers are allowed\n");
    exit(1);
  }
  if (numOfIntegers < MIN_INTEGERS) {
    printf("Invalid number of integers\n");
    printf("Minimum 1000 integers are allowed\n");
    exit(1);
  }
  if(numOfChildren > MAX_CHILDREN) {
    printf("Invalid number of children\n");
    printf("Maximum 50 children are allowed\n");
    exit(1);
  }
  if(numOfChildren < MIN_CHILDREN) {
    printf("Invalid number of children\n");
    printf("Minimum 1 children are allowed\n");
    exit(1);
  }
}

int main(int argc, char **argv) {

  struct Queue* mainQueue = createQueue();
  struct Queue* bufferQueue = createQueue();

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
  for (int i = 2; i <= numOfIntegers; i++) {
    enqueue(mainQueue, i);
  }
  enqueue(mainQueue, -1);

  int i;
  for (i = 0; i <= numOfChildren; i++) {
    childProcesses[i] = fork();
    if (childProcesses[i] < 0) {
      fprintf(stderr, "Fork failed\n");
      exit(1);
    }
    else if (childProcesses[i] == 0) {
      int numberToRead;
      int primeNumber = END_OF_DATA;

      // Printer process
      if (i == numOfChildren) {
        while(1) {
          if (read(printerPipe[READ_END], &numberToRead, sizeof(numberToRead)) > 0 && numberToRead) {
            // Terminate the child if kill signal is received
            if (numberToRead == KILL_SIGNAL){
              exit(0);
            }
            printf("%d\n", numberToRead);
          }
        }
      }
      // Child process
      else {
        while(1) {

          // Read the number from pipe
          if (read(childrenFDs[i][READ_END], &numberToRead, sizeof(numberToRead)) > 0) {
            if (numberToRead == KILL_SIGNAL){
              write(childrenFDs[i+1][WRITE_END], &numberToRead, sizeof(numberToRead));
              exit(0);
            }

            // First number of the series is read, it is the prime number
            if (numberToRead == END_OF_DATA) {
              primeNumber = END_OF_DATA;
              write(childrenFDs[i+1][WRITE_END], &numberToRead, sizeof(numberToRead));
            }
            else if (primeNumber == END_OF_DATA && numberToRead != END_OF_DATA){
              primeNumber = numberToRead;
              // Send prime number to printerPipe
              write(printerPipe[WRITE_END], &primeNumber, sizeof(primeNumber));
            }
            // A number from the middle is read, send it to next child
            // If it is not a multiple of last prime number
            else if (numberToRead % primeNumber != 0){
              write(childrenFDs[i+1][WRITE_END], &numberToRead, sizeof(numberToRead));
            }
          }
        }
      }
    }
  }

  // Parent process
  if (getpid() == mainPid) {
    int numberToRead;
    int numberToWrite;

    while (1) {
      if (!isEmpty(mainQueue)) {
        numberToWrite = dequeue(mainQueue);
        write(childrenFDs[0][WRITE_END], &numberToWrite, sizeof(numberToWrite));
      }

      if (read(childrenFDs[numOfChildren][READ_END], &numberToRead, sizeof(numberToRead)) > 0){
        enqueue(bufferQueue, numberToRead);
        if (numberToRead == END_OF_DATA) {

          if (bufferQueue->head->data == END_OF_DATA) {
            numberToWrite = KILL_SIGNAL;
            write(childrenFDs[0][WRITE_END], &numberToWrite, sizeof(numberToWrite));
            write(printerPipe[WRITE_END], &numberToWrite, sizeof(numberToWrite));
            break;
          }
          struct Queue *temp = mainQueue;
          mainQueue = bufferQueue;
          if(isEmpty(temp)){
            bufferQueue = temp;
          }
          else {
            bufferQueue = createQueue();
          }
        }
      }
    }
    for (int i = 0; i <= numOfChildren; i++) {
      wait(NULL);
    }

    free(mainQueue);
    while(!isEmpty(bufferQueue)){
      dequeue(bufferQueue);
    }
    free(bufferQueue);
    exit(0);
  }

  return 0;
}
