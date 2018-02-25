#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "string.h"
#include "sys/types.h"
#include "sys/wait.h"

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

struct Node* dequeue(struct Queue* queue) {
  if (queue->head == NULL) {
    return NULL;
  }
  else {
    struct Node* node = queue->head;
    queue->head = queue->head->next;

    if (queue->head == NULL) {
      queue->tail = NULL;
    }

    return node;
  }
}

int isEmpty(struct Queue* queue) {
  return queue->head == NULL;
}

/*************************
*****PRIME CALCULATOR*****
*************************/

int main(int argc, char **argv) {

  struct Queue *mainQueue = createQueue();

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

  for (int i = 2; i <= numOfIntegers; i++) {
    enqueue(mainQueue, i);
  }
  enqueue(mainQueue, -1);

  pid_t childProcesses[numOfChildren];

  int childrenFDs[numOfChildren + 1][2];
  for (int i = 0; i < numOfChildren + 1; i++) {
    if (pipe(childrenFDs[i]) == -1){
      printf("Pipe opening failed.\n");
      exit(1);
    }
  }

  for (int i = 0; i < numOfChildren; i++) {
    childProcesses[i] = fork();
    if (childProcesses[i] < 0) {
      fprintf(stderr, "Fork failed");
      exit(-1);
    }
    else if (childProcesses[i] == 0) { // Child process
      printf("I am a child process %d\n", childProcesses[i]);

      int numberToRead;
      if (read(childrenFDs[i][READ_END], &numberToRead, 20) == -1) {
        printf("Error while reading");
        exit(1);
      }
      printf("%d\n", numberToRead);

      if (write(childrenFDs[i+1][WRITE_END], "hi1", 4) != 4) {
        printf("Error while writing");
      }
    }
    else { // Parent process
      int numberToSent = (dequeue(mainQueue))->data;
      if (write(childrenFDs[0][WRITE_END], &numberToSent, sizeof(numberToSent)) != sizeof(numberToSent)) {
        printf("Error while writing");
      }

      wait(NULL);
      printf("Children complete, pid: %d\n", childProcesses[i]);
      exit(0);
    }
  }

  return 0;
}
