#include "stdio.h"
#include "stdlib.h"
#include "unistd.h"
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
struct item {
  int data;
};

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

int receiveNumber(mqd_t queueDescriptor) {
  mqd_t mq = queueDescriptor;
  struct mq_attr mq_attr;
  struct item *itemptr;
  int buflen;
  char *bufptr;
  int n;

  mq_getattr(mq, &mq_attr);

  buflen = mq_attr.mq_msgsize;
  bufptr = (char *) malloc(buflen);

  n = mq_receive(mq, (char *) bufptr, buflen, NULL);
  if (n == -1) {
    printf("failed to receive");
    exit(1);
  }
  itemptr = (struct item *) bufptr;

  free(bufptr);
  mq_close(mq);

  return itemptr->data;
}

void sendNumber(mqd_t queueDescriptor, int data) {
  struct item itemToSend;
  itemToSend.data = data;
  if (mq_send(queueDescriptor, (char *) &itemToSend, sizeof(struct item), 0) == -1) {
    printf("mq_send failed\n");
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

  mqd_t childQueues[numOfChildren];
  mqd_t printerQueue;

  // Check if the arguments are correct
  //checkArguments(argc, numOfIntegers, numOfChildren);

  // Insert all numbers to main QUEUE
  for (int i = 2; i <= numOfIntegers; i++) {
    enqueue(mainQueue, i);
  }
  enqueue(mainQueue, -1);

  // Open message queues
  for (int i = 0; i < numOfChildren + 1; i++) {
    char msgQueueName[20];
    sprintf(msgQueueName, "/childQueue%d", i);
    childQueues[i] = mq_open(msgQueueName, O_RDWR | O_CREAT, 0666, NULL);
    if (childQueues[i] == -1) {
      perror("Failed to create message queue\n");
      exit(1);
    }
  }

  printerQueue = mq_open("/printerQueue", O_RDWR | O_CREAT, 0666, NULL);
  if(printerQueue == -1) {
    perror("Failed to create message queue\n");
    exit(1);
  }

  //Fork child processes
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
          mqd_t queueDescriptor = mq_open("/printerQueue", O_RDWR);
          if (queueDescriptor == -1) {
            printf("can not open message queue");
            exit(1);
          }
          numberToRead = receiveNumber(queueDescriptor);
          printf("%d\n", numberToRead);
        }
      }
      //Child process
      else {
        while(1) {
          char msgQueueName[20];
          sprintf(msgQueueName, "/childQueue%d", i);
          mqd_t queueDescriptor = mq_open(msgQueueName, O_RDWR);
          if (queueDescriptor == -1) {
            printf("can not open message queue");
            exit(1);
          }
          numberToRead = receiveNumber(queueDescriptor);
          if (numberToRead == END_OF_DATA) {
            primeNumber = END_OF_DATA;
            sendNumber(childQueues[i+1], numberToRead);
          }
          else if (primeNumber == END_OF_DATA && numberToRead != END_OF_DATA){
            primeNumber = numberToRead;
            // Send prime number to printerQueue
            sendNumber(printerQueue, primeNumber);
          }
          // A number from the middle is read, send it to next child
          // If it is not a multiple of last prime number
          else if (numberToRead % primeNumber != 0){
            sendNumber(childQueues[i+1], numberToRead);
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
        sendNumber(childQueues[0], numberToWrite);
      }

      char msgQueueName[20];
      sprintf(msgQueueName, "/childQueue%d", numOfChildren);
      mqd_t queueDescriptor = mq_open(msgQueueName, O_RDWR);
      if (queueDescriptor == -1) {
        printf("can not open message queue");
        exit(1);
      }

      numberToRead = receiveNumber(queueDescriptor);
      enqueue(bufferQueue, numberToRead);
      if (numberToRead == END_OF_DATA) {

        if (bufferQueue->head->data == END_OF_DATA) {
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
    free(mainQueue);
    while(!isEmpty(bufferQueue)){
      dequeue(bufferQueue);
    }
    free(bufferQueue);
    exit(0);
  }

  return 0;
}
