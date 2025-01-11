#ifndef WBQ_H
#define WBQ_H

// Structs and methods for WorkBalancerQueue

// **********************************************************

typedef struct WorkBalancerQueue WorkBalancerQueue;

typedef struct ThreadArguments {
    WorkBalancerQueue* q;
    int id;
} ThreadArguments;

typedef struct Task {
    char* task_id;
    int task_duration;
	double cache_warmed_up;
	WorkBalancerQueue* owner;
} Task;

typedef struct QueueNode {
    Task *task; // Task
    struct QueueNode *nextPtr; // Showing next on the queue
    struct QueueNode *prevPtr; // Showing previous on the queue
} QueueNode;

struct WorkBalancerQueue {
    // We do not need to use typedef since prof already did on #9
    // ---MBE--- ADD Michael scott, dummy/head/tail
    QueueNode *head; // Head of the queue
    QueueNode *beforeTail; // Before tail of the queue
    QueueNode *tail; // Tail of the queue
    
    // Add mutexes for each queue
    pthread_mutex_t head_lock;
    pthread_mutex_t tail_lock;
    pthread_mutex_t size_lock;
    pthread_mutex_t mark_lock;

    int queue_size; // Size of the queue
    int owner_id; // Owner id of the queue == thread id
    
    int high_watermark; // Threshold for high-watermark
};

// **********************************************************

// WorkBalancerQueue API **********************************************************
void submitTask(WorkBalancerQueue* q, Task* _task);
Task* fetchTask(WorkBalancerQueue* q);
Task* fetchTaskFromOthers(WorkBalancerQueue* q);
int learnSize(WorkBalancerQueue* q);
void updateQueueWatermarks(WorkBalancerQueue* q, WorkBalancerQueue** all_queues, int num_cores);

// **********************************************************

void executeJob(Task* task, WorkBalancerQueue* my_queue, int my_id );

void* processJobs(void* arg);
void initSharedVariables();
#endif