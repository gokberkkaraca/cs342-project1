#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
#include "fcntl.h"
#include "mqueue.h"

#define END_OF_DATA -1
#define MAX_INTEGERS 1000000
#define MIN_INTEGERS 1000
#define MAX_CHILDREN 5
#define MIN_CHILDREN 1

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
    printf("Maximum 5 children are allowed\n");
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

  // Check if the arguments are correct
  checkArguments(argc, numOfIntegers, numOfChildren);

}
