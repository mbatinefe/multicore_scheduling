#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include "wbq.h"

// WorkBalancerQueue implementation. 

void submitTask(WorkBalancerQueue* q, Task* _task) {
    // My Code

    // Allocate memory for new node
    struct QueueNode *newQueueNode = malloc(sizeof(struct QueueNode));
    newQueueNode->task = _task;
    newQueueNode->nextPtr = NULL;
    newQueueNode->prevPtr = NULL;

    // Assert if we have memory
    assert (newQueueNode != NULL);

    pthread_mutex_lock(&q->tail_lock);
    // ----------- CRITICAL SECTION : START ----------------

    // Sample implementation of structure
    //          [DUMMY] -- nextPtr --> [TASK1] -- nextPtr --> [TASK2] -- nextPtr --> [TASK3] 
    // NULL <-- prevPtr           <-- prevPtr            <-- prevPtr             <-- prevPtr
    //           head                                     beforeTail                  tail

    q->tail->nextPtr = newQueueNode; // Show tail's next pointer to "new tail node"
    q->beforeTail = q->tail; // Update beforeTail as new tail
    q->tail = q->tail->nextPtr; // Update tail as "new tail node"
    q->tail->prevPtr = q->beforeTail; // Update "new tail node"s previous pointer as beforeTail
    q->queue_size++; // Increase queue size
    _task->owner = q; // Assign owner of the task as this queue
    // ------------ CRITICAL SECTION : END -----------------
    pthread_mutex_unlock(&q->tail_lock);
}

Task* fetchTask(WorkBalancerQueue* q) {
    // My Code

    pthread_mutex_lock(&q->head_lock);
    // ----------- CRITICAL SECTION : START ----------------
    if (q->tail == q->head) {
        // If head and tail are same, they are on dummy node and queue is empty
         // ------------ CRITICAL SECTION : END -----------------
        pthread_mutex_unlock(&q->head_lock);
        return NULL;
    } 

    // Show the first head -->> head(dummy)'s nextPtr
    struct QueueNode *first_head = q->head->nextPtr;
    // Next one is the second node from head. Remember NULL if there is only 1 task node
    struct QueueNode *second_head = q->head->nextPtr->nextPtr;

    // We have only 1 task in queue
    if(first_head == q->tail){
        q->tail = q->head; // Update tail as head(dummy)
        Task *task = first_head->task; // Get the task
        q->head->nextPtr = NULL; // Update head(dummy)'s nextPtr as NULL
        q->queue_size--; // Decrease queue size
        
        // ------------ CRITICAL SECTION : END -----------------
        pthread_mutex_unlock(&q->head_lock);
        free(first_head);
        return task;
    }

    // Afterwards are for 2 or more tasks
    Task *task = first_head->task; // Get the task
    q->head->nextPtr = second_head; // Update head(dummy)'s nextPtr as second node
    second_head->prevPtr = q->head; // Update second head's prevPtr as head(dummy)
    q->queue_size--; // Decrease queue size

    if(first_head == q->beforeTail) {
        // Meaning we have 2 task in queque -> we need to update "beforeTail"
        q->beforeTail = q->tail->prevPtr;
    }

    // ------------ CRITICAL SECTION : END -----------------
    pthread_mutex_unlock(&q->head_lock);
    // free(first_head); // We can free it here but no need, our aim is not that
    return task;
}

Task* fetchTaskFromOthers(WorkBalancerQueue* q) {
    // My Code

    pthread_mutex_lock(&q->tail_lock);
    // ----------- CRITICAL SECTION : START ----------------

    // Queque is empty if tail and head pointing at the dummy node
    if (q->tail == q->head) {
        // ------------ CRITICAL SECTION : END -----------------
        pthread_mutex_unlock(&q->tail_lock);
        return NULL;
    }

    // Followings are for 1 or more tasks
    struct QueueNode *old_tail= q->tail; // Assign tail as old tail
    Task *task = old_tail->task; // Get the task
    
    q->tail = q->beforeTail; // Update tail as beforeTail
    q->tail->nextPtr = NULL; // Update tail's nextPtr as NULL
    q->queue_size--; // Decrease queue size
    // ------------ CRITICAL SECTION : END -----------------
    pthread_mutex_unlock(&q->tail_lock);
    //free(old_tail); // We can free it here but no need, our aim is not that
    return task;
}

// Following function is to learn the size of the queue atomically
int learnSize(WorkBalancerQueue* q){
    int size;
    pthread_mutex_lock(&q->size_lock);
    // ----------- CRITICAL SECTION : START ----------------
    size = q->queue_size;
    // ------------ CRITICAL SECTION : END -----------------
    pthread_mutex_unlock(&q->size_lock);
    return size;
}

// Following function is to update the high-watermark of the queue with arranged algorithm
// Please see the report for more detailed explanation and activate print statements to further investigate
void updateQueueWatermarks(WorkBalancerQueue* q, WorkBalancerQueue** all_queues, int num_cores) {
    int total_tasks = 0; // Number of total tasks in all queues
    int empty_cores = 0; // Number of empty cores that is not working
    int working_cores = 0; // Number of cores that is working
    double threshold = 0.9; // Default threshold for high-watermark

    pthread_mutex_lock(&q->mark_lock);
    // ----------- CRITICAL SECTION : START ----------------
    
    // Travel through all queues and learn the size of the queue
    for (int i = 0; i < num_cores; i++) {
        int size = learnSize(all_queues[i]);
        if (size > 0) {
            // If there is task, meaning they are working
            total_tasks += size;
            working_cores++;
        } else{
            // If there is no task, meaning they are not working
            empty_cores++;
        }
    }
    
    // If all cores are empty it means, we do not need to do anything, just skip
    if(empty_cores != num_cores){
        // Average of tasks per core
        // REMINDER : We do not check how much each task has weight, we just count them
        float avg = total_tasks / num_cores;
    
        // printf("Average is %.2f, total cores:%d, work: %d, empty: %d", avg, num_cores, working_cores, empty_cores);
        
        // If we have more empty cores than working cores, we decrease the threshold
        // It means that we will set high-watermark lower than it suppose to
        // As a result, it will let more cores steal
        if(empty_cores>working_cores){
            threshold = 0.5;
        }

        for (int i = 0; i < num_cores; i++) {
            all_queues[i]->high_watermark = (int)(avg * threshold);
        }

        // printf(" --> Arranged high-watermark as %d\n", all_queues[0]->high_watermark);
    }

    pthread_mutex_unlock(&q->mark_lock);
}
