#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_PARAMS 12
#define EVENT_DESCRIPTION 30

typedef struct event{
    int jobNum;
    char description[EVENT_DESCRIPTION];
    int time;
} event;

typedef struct QNode{
    struct event *event;
    struct QNode *next;
} QNode;

typedef struct queue{
    struct QNode *head;
    struct QNode *tail;
    int size;
} queue;


//Function prototypes
struct QNode * newNode(event *k);
struct queue * createQueue();
void enqueue (queue *q, event *k);
struct QNode * dequeue (queue *q);
struct event * newEvent (int x, char arr[EVENT_DESCRIPTION], int y);
int size(queue *q);
void enqueuePriority (queue *q, event *k);
void printEvent(event *k, FILE *fout);

void simulation(int confVar[NUM_PARAMS], FILE *fout);

void disk1Handler(int confVar[NUM_PARAMS], QNode *toBeProcessed, queue *eventQueue, queue *CPU, queue *disk1, int *CPUIdlePtr, int *Disk1IdlePtr, int *maxCPUPtr, int *totalDisk1Ptr,
                  int *Disk1ProcessPtr, int *CPUResponsePtr, int *Disk1ResponsePtr, int *totalCPUResponsePtr, int *totalDisk1ResponsePtr);
void disk2Handler(int confVar[NUM_PARAMS], QNode *toBeProcessed, queue *eventQueue, queue *CPU, queue *disk2, int *CPUIdlePtr, int *Disk2IdlePtr, int *maxCPUPtr, int *totalDisk2Ptr,
                  int *Disk2ProcessPtr, int *CPUResponsePtr, int *Disk2ResponsePtr, int *totalCPUResponsePtr, int *totalDisk2ResponsePtr);

void arrivalHandler(int confVar[NUM_PARAMS], QNode *toBeProcessed, queue *eventQueue, queue *CPU, int *CPUIdlePtr, int *maxCPUPtr, int *CPUResponsePtr, int *totalCPUResponsePtr);

void CPUHandler(int confVar[NUM_PARAMS], QNode *toBeProcessed, queue *eventQueue,queue *CPU,queue *disk1,queue *disk2, int *CPUIdlePtr, int *Disk1IdlePtr, int *Disk2IdlePtr,
                int *maxDisk1Ptr, int *maxDisk2Ptr, int *totalCPUPtr, int *CPUProcessPtr, int *CPUResponsePtr, int *Disk1ResponsePtr, int *Disk2ResponsePtr,
                int *totalCPUResponsePtr, int *totalDisk1ResponsePtr, int *totalDisk2ResponsePtr);


enum {SEED, INIT_TIME, FIN_TIME, ARRIVE_MIN, ARRIVE_MAX, QUIT_PROB, CPU_MIN, CPU_MAX, DISK1_MIN, DISK1_MAX, DISK2_MIN, DISK2_MAX};

int main()
{
	
	// Input data from config.txt to the program
    int confVar[NUM_PARAMS] = {};
    const char *conf_types[NUM_PARAMS] = {"SEED", "INIT_TIME", "FIN_TIME", "ARRIVE_MIN" , "ARRIVE_MAX", "QUIT_PROB", "CPU_MIN", "CPU_MAX", "DISK1_MIN", "DISK1_MAX", "DISK2_MIN", "DISK2_MAX"};

    FILE *fin = fopen("config.txt", "r");

    if (fin == NULL)
    {
        printf("Cannot open input file \n");
        exit(0);
    }

    char line[20];
    char search_str[20];
    int i = 0; //get value

    while(fgets(line, 20, fin)!= NULL){
            strcpy( search_str, conf_types[i] );
            strcat( search_str, " %d\n" );
            sscanf( line, search_str, &confVar[i] );
            i++;
    }

    i = 0;

	// Output the record to another file called log.txt
    FILE *fout = fopen("log.txt", "w");
    if (fout == NULL) {
        printf("Cannot open output file \n");
        exit(0);
    }

    for(;i<NUM_PARAMS;i++){
        fprintf(fout, "%s %s %d\n", conf_types[i], ": ", confVar[i]);
    }
    simulation(confVar, fout);

    fclose(fin);
    fclose(fout);
}

// Simulate quêu for event, CPU, disk1 & disk 2
void simulation(int confVar[NUM_PARAMS], FILE *fout){
    queue *eventQueue = createQueue();
    queue *CPU = createQueue();
    queue *disk1 = createQueue();
    queue *disk2 = createQueue();

    srand((int) confVar[SEED]);

    int currentTime = 0;

	// Create boolean value to be considered later
    int CPUIsIdle = 1;
    int Disk1IsIdle = 1;
    int Disk2IsIdle = 1;

	// Set 0 as initial value
    int maxCPUSize = 0;
    int maxDisk1Size = 0;
    int maxDisk2Size = 0;

	// Set 0 as initial value
    int totalCPUSize = 0;
    int totalDisk1Size = 0;
    int totalDisk2Size = 0;

	// Set 0 as initial value
    int totalCPUProcesses = 0;
    int totalDisk1Processes = 0;
    int totalDisk2Processes = 0;

	// Set 0 as initial value
    int maxCPUResponse = 0;
    int maxDisk1Response = 0;
    int maxDisk2Response = 0;

	// Set 0 as initial value
    int totalCPUResponse = 0;
    int totalDisk1Response = 0;
    int totalDisk2Response = 0;

    enqueuePriority(eventQueue,newEvent(1,"arrives",0));

	// Consider the busy of the queue to scheduling the event
    while((currentTime < confVar[FIN_TIME])&&(size(eventQueue)>0)){
            QNode *toBeProcessed = dequeue(eventQueue);
            currentTime = toBeProcessed->event->time;

            printEvent(toBeProcessed->event, fout);
            if(strcmp(toBeProcessed->event->description,"arrives")==0){
                arrivalHandler(confVar, toBeProcessed, eventQueue, CPU, &CPUIsIdle, &maxCPUSize, &maxCPUResponse, &totalCPUResponse);
            } else if (strcmp(toBeProcessed->event->description,"finishes at CPU")==0){
                CPUHandler(confVar, toBeProcessed, eventQueue, CPU, disk1, disk2, &CPUIsIdle, &Disk1IsIdle, &Disk2IsIdle, &maxDisk1Size, &maxDisk2Size, &totalCPUSize,
                            &totalCPUProcesses, &maxCPUResponse, &maxDisk1Response, &maxDisk2Response, &totalCPUResponse, &totalDisk1Response, &totalDisk2Response);
            } else if (strcmp(toBeProcessed->event->description,"finishes at Disk 1")==0){
                disk1Handler(confVar, toBeProcessed, eventQueue, CPU, disk1, &CPUIsIdle, &Disk1IsIdle, &maxCPUSize, &totalDisk1Size,
                             &totalDisk1Processes, &maxCPUResponse, &maxDisk1Response, &totalCPUResponse, &totalDisk1Response);
            } else if (strcmp(toBeProcessed->event->description,"finishes at Disk 2")==0){
                disk2Handler(confVar, toBeProcessed, eventQueue, CPU, disk2, &CPUIsIdle, &Disk2IsIdle, &maxCPUSize, &totalDisk2Size,
                             &totalDisk2Processes, &maxCPUResponse, &maxDisk2Response, &totalCPUResponse, &totalDisk2Response);
            }
    }
    fprintf(fout, "%-5d %s\n", confVar[FIN_TIME], "Simulation finishes");

	// Print out recorded data to the screen
    printf("%s %d\n", "Max CPU Queue Size: ", maxCPUSize);
    if(totalCPUProcesses>0){
    printf("%s %d\n", "Average CPU Queue Size: ", totalCPUSize/totalCPUProcesses);
    printf("%s %d\n", "Average CPU Response Time: ", totalCPUResponse/totalCPUProcesses);
    } else {
    printf("%s %d\n", "Max CPU Queue Size: ", maxCPUSize);
    printf("%s %d\n", "Max CPU Response Time: ", maxCPUResponse);
    }
    printf("%s %d\n", "Max CPU Response Time: ", maxCPUResponse);
    printf("%s %d\n\n", "Total CPU Processes: ", totalCPUProcesses);


    printf("%s %d\n", "Max Disk 1 Queue Size: ", maxDisk1Size);
    if(totalDisk1Processes>0){
    printf("%s %d\n", "Average Disk 1 Queue Size: ", totalDisk1Size/totalDisk1Processes);
    printf("%s %d\n", "Average Disk 1 Response Time: ", totalDisk1Response/totalDisk1Processes);
    } else {
    printf("%s %d\n", "Max Disk 1 Queue Size: ", maxDisk1Size);
    printf("%s %d\n", "Max Disk 1 Response Time: ", maxDisk1Response);
    }
    printf("%s %d\n", "Max Disk 1 Response Time: ", maxDisk1Response);
    printf("%s %d\n\n", "Total Disk 1 Processes: ", totalDisk1Processes);


    printf("%s %d\n", "Max Disk 2 Queue Size: ", maxDisk2Size);
    if(totalDisk2Processes>0){
    printf("%s %d\n", "Average Disk 2 Queue Size: ", totalDisk2Size/totalDisk2Processes);
    printf("%s %d\n", "Average Disk 2 Response Time: ", totalDisk2Response/totalDisk2Processes);
    } else {
    printf("%s %d\n", "Max Disk 2 Queue Size: ", maxDisk2Size);
    printf("%s %d\n", "Max Disk 2 Response Time: ", maxDisk2Response);
    }
    printf("%s %d\n", "Max Disk 2 Response Time: ", maxDisk2Response);
    printf("%s %d\n", "Total Disk 2 Processes: ", totalDisk2Processes);
}

	//Create a new arrival event
void arrivalHandler(int confVar[NUM_PARAMS],QNode *toBeProcessed, queue *eventQueue, queue *CPU, int *CPUIdlePtr, int *maxCPUPtr, int *CPUResponsePtr, int *totalCPUResponsePtr){
    int currentTime = toBeProcessed->event->time;
    int randomTimeArrival = (rand()%(confVar[ARRIVE_MAX]-confVar[ARRIVE_MIN])+confVar[ARRIVE_MIN]);

    if(currentTime+randomTimeArrival<confVar[FIN_TIME]){
        event *jobArrives = newEvent(toBeProcessed->event->jobNum+1, "arrives", currentTime+randomTimeArrival);
        enqueuePriority(eventQueue, jobArrives);
    }

    //Create a CPU event
    int randomTimeCPU = rand()%(confVar[CPU_MAX]-confVar[CPU_MIN])+confVar[CPU_MIN];

    if(currentTime+randomTimeCPU<confVar[FIN_TIME]){
        event *finishedAtCPU = newEvent(toBeProcessed->event->jobNum, "finishes at CPU", currentTime+randomTimeCPU);
        if(*CPUIdlePtr){
            enqueuePriority(eventQueue, finishedAtCPU);
            *CPUIdlePtr=0;
            if (*CPUResponsePtr < randomTimeCPU){
                *CPUResponsePtr = randomTimeCPU;
            }
            *totalCPUResponsePtr+=randomTimeCPU;
        } else {
            enqueue(CPU, finishedAtCPU);
            if(*maxCPUPtr<size(CPU)){
                *maxCPUPtr=size(CPU);
            }
        }
    }
}

	
void CPUHandler(int confVar[NUM_PARAMS], QNode *toBeProcessed, queue *eventQueue,queue *CPU,queue *disk1,queue *disk2, int *CPUIdlePtr, int *Disk1IdlePtr, int *Disk2IdlePtr, int *maxDisk1Ptr, int *maxDisk2Ptr, int *totalCPUPtr,
                int *CPUProcessPtr, int *CPUResponsePtr, int *Disk1ResponsePtr, int *Disk2ResponsePtr, int *totalCPUResponsePtr, int *totalDisk1ResponsePtr, int *totalDisk2ResponsePtr){
    int jobNum = toBeProcessed->event->jobNum;
    int time = toBeProcessed->event->time;

    *CPUProcessPtr+=1;
    *totalCPUPtr+=size(CPU);

	// When event is processed and finished at CPU
    if(rand()%100+1<confVar[QUIT_PROB]){
        enqueuePriority(eventQueue,newEvent(jobNum, "finishes", time));
    } else {
    // When event is not processed and finished at CPU, it's moved to disk1 
    if((disk1->size<disk2->size)||((disk1->size==disk2->size)&&(rand()%100+1>50))){
        int randomTimeDisk = rand()%(confVar[DISK1_MAX]-confVar[DISK1_MIN])+confVar[DISK1_MIN];

        if(time+randomTimeDisk<confVar[FIN_TIME]){
            event *finishedAtDisk1 = newEvent(jobNum,"finishes at Disk 1", time+randomTimeDisk);
            if(*Disk1IdlePtr){
                enqueuePriority(eventQueue,finishedAtDisk1);
                *Disk1IdlePtr = 0;
                if (*Disk1ResponsePtr < randomTimeDisk){
                    *Disk1ResponsePtr = randomTimeDisk;
                }
                *totalDisk1ResponsePtr+=randomTimeDisk;
            } else {
                enqueue(disk1,finishedAtDisk1);
                if(*maxDisk1Ptr<size(disk1)){
                    *maxDisk1Ptr=size(disk1);
                }
            }
        }
    } else {
    	// When event is not processed and finished at CPU, it's moved to disk2 
        int randomTimeDisk = rand()%(confVar[DISK2_MAX]-confVar[DISK2_MIN])+confVar[DISK2_MIN];

        if(time+randomTimeDisk<confVar[FIN_TIME]){
            event *finishedAtDisk2 = newEvent(jobNum,"finishes at Disk 2", time+randomTimeDisk);
            if(*Disk2IdlePtr){
                enqueuePriority(eventQueue,finishedAtDisk2);
                *Disk2IdlePtr = 0;
                if (*Disk2ResponsePtr < randomTimeDisk){
                    *Disk2ResponsePtr = randomTimeDisk;
                }
                *totalDisk2ResponsePtr+=randomTimeDisk;
            } else {
                enqueue(disk2,finishedAtDisk2);
                if(*maxDisk2Ptr<size(disk2)){
                    *maxDisk2Ptr=size(disk2);
                }
            }
        }
    }
    }

    if(CPU->size==0){
        *CPUIdlePtr=1;
    } else {
    	// when the queue is free, move the one from the queue to be processed
        event *next = (dequeue(CPU))->event;
        int timeNext = toBeProcessed->event->time;
        int jobNext = next->jobNum;
        int randomTimeCPU = rand()%(confVar[CPU_MAX]-confVar[CPU_MIN])+confVar[CPU_MIN];
        if(timeNext+randomTimeCPU<confVar[FIN_TIME]){
        event *finishedAtCPU = newEvent(jobNext, "finishes at CPU", timeNext+randomTimeCPU);
        enqueuePriority(eventQueue, finishedAtCPU);
        if (*CPUResponsePtr < randomTimeCPU){
            *CPUResponsePtr = randomTimeCPU;
        }
        *totalCPUResponsePtr+=randomTimeCPU;
        }
    }
}

	// Same as CPUHandler but for disk1
void disk1Handler(int confVar[NUM_PARAMS], QNode *toBeProcessed, queue *eventQueue, queue *CPU, queue *disk1, int *CPUIdlePtr, int *Disk1IdlePtr, int *maxCPUPtr, int *totalDisk1Ptr,
                  int *Disk1ProcessPtr, int *CPUResponsePtr, int *Disk1ResponsePtr, int *totalCPUResponsePtr, int *totalDisk1ResponsePtr){
    int jobNum = toBeProcessed->event->jobNum;
    int time = toBeProcessed->event->time;
    int randomTimeCPU = rand()%(confVar[CPU_MAX]-confVar[CPU_MIN])+confVar[CPU_MIN];

    if(time+randomTimeCPU<confVar[FIN_TIME]){
        event *finishedAtCPU = newEvent(jobNum, "finishes at CPU", time+randomTimeCPU);
        if(*CPUIdlePtr){
            enqueuePriority(eventQueue, finishedAtCPU);
            *CPUIdlePtr = 0;
            if (*CPUResponsePtr < randomTimeCPU){
                *CPUResponsePtr = randomTimeCPU;
            }
            *totalCPUResponsePtr+=randomTimeCPU;
        } else {
            enqueue(CPU, finishedAtCPU);
            if(*maxCPUPtr<size(CPU)){
                *maxCPUPtr=size(CPU);
            }
        }
    }

    *Disk1ProcessPtr+=1;
    *totalDisk1Ptr+=size(disk1);

    if(size(disk1)==0){
        *Disk1IdlePtr = 1;
    } else {
        event *next = (dequeue(disk1))->event;
        int timeNext = toBeProcessed->event->time;
        int jobNext = next->jobNum;
        int randomTimeDisk = rand()%(confVar[DISK1_MAX]-confVar[DISK1_MIN])+confVar[DISK1_MIN];
        if(timeNext+randomTimeDisk<confVar[FIN_TIME]){
            event *finishedAtDisk = newEvent(jobNext, "finishes at Disk 1", timeNext+randomTimeDisk);
            enqueuePriority(eventQueue, finishedAtDisk);
            if (*Disk1ResponsePtr < randomTimeDisk){
                *Disk1ResponsePtr = randomTimeDisk;
            }
            *totalDisk1ResponsePtr+=randomTimeDisk;
        }
    }
}

	// Same as CPUHandler but for disk2
void disk2Handler(int confVar[NUM_PARAMS], QNode *toBeProcessed, queue *eventQueue, queue *CPU, queue *disk2, int *CPUIdlePtr, int *Disk2IdlePtr, int *maxCPUPtr, int *totalDisk2Ptr,
                  int *Disk2ProcessPtr, int *CPUResponsePtr, int *Disk2ResponsePtr, int *totalCPUResponsePtr, int *totalDisk2ResponsePtr){
    int jobNum = toBeProcessed->event->jobNum;
    int time = toBeProcessed->event->time;
    int randomTimeCPU = rand()%(confVar[CPU_MAX]-confVar[CPU_MIN])+confVar[CPU_MIN];
    if(time+randomTimeCPU<confVar[FIN_TIME]){
        event *finishedAtCPU = newEvent(jobNum, "finishes at CPU", time+randomTimeCPU);
        if(*CPUIdlePtr){
            enqueuePriority(eventQueue, finishedAtCPU);
            *CPUIdlePtr = 0;
        } else {
            enqueue(CPU, finishedAtCPU);
            if(*maxCPUPtr<size(CPU)){
                *maxCPUPtr=size(CPU);
            }
        }
    }

    *Disk2ProcessPtr+=1;
    *totalDisk2Ptr+=size(disk2);

    if(size(disk2)==0){
        *Disk2IdlePtr = 1;
    } else {
        event *next = (dequeue(disk2))->event;
        int timeNext = toBeProcessed->event->time;
        int jobNext = next->jobNum;
        int randomTimeDisk = rand()%(confVar[DISK2_MAX]-confVar[DISK2_MIN])+confVar[DISK2_MIN];
        if(timeNext+randomTimeDisk<confVar[FIN_TIME]){
            event *finishedAtDisk = newEvent(jobNext, "finishes at Disk 2", timeNext+randomTimeDisk);
            enqueuePriority(eventQueue, finishedAtDisk);
            if (*Disk2ResponsePtr < randomTimeDisk){
                *Disk2ResponsePtr = randomTimeDisk;
            }
            *totalDisk2ResponsePtr+=randomTimeDisk;
        }
    }
}

void printEvent(event *k, FILE *fout){
    fprintf(fout,"%-5d %s %-4d %s\n", k->time, "Job", k->jobNum, k->description);
}

struct event * newEvent (int x, char arr[EVENT_DESCRIPTION], int y){
    event *temp = (event*)malloc(sizeof(event));
    temp->jobNum = x;
    strcpy(temp->description, arr);
    temp->time = y;
    return temp;
}

struct QNode * newNode(event *k){
    QNode *temp = (QNode*)malloc(sizeof(QNode));
    temp->event = k;
    temp->next = NULL;
    return temp;
}

struct queue * createQueue()
{
    queue *q = (queue*)malloc(sizeof(queue));
    q->head = q->tail = NULL;
    q->size = 0;
    return q;
}

	// Use FIFO to create queue priority
void enqueuePriority (queue *q, event *k){
    QNode *temp = newNode(k);
    if(q->tail==NULL){
        q->head=q->tail=temp;
    } else {
        if(q->head->event->time> temp->event->time){
        temp->next=q->head;
        q->head = temp;
        } else {
            QNode *prev = q->head;
            QNode *current = prev->next;
            while(prev!=NULL&&current!=NULL){
                if(temp->event->time<current->event->time){
                    prev->next=temp;
                    temp->next=current;
                    q->size++;
                    return;
                } else {
                    prev = prev->next;
                    current = prev->next;
                }
            }
            q->tail->next=temp;
            q->tail=temp;
        }
    }
    q->size++;
}

int size(queue *q){
    return q->size;
}

void enqueue (queue *q, event *k){
    struct QNode *temp = newNode(k);
    if(q->tail==NULL){
        q->head=q->tail=temp;
    } else {
        q->tail->next=temp;
        q->tail=temp;
    }
    q->size++;
}

struct QNode * dequeue (queue *q) {
    if(q->head==NULL){
        return NULL;
    }
    QNode *temp = q->head;
    q->head = q->head->next;

    if (q->head == NULL){
       q->tail = NULL;
    }
    q->size--;
    return temp;
}
