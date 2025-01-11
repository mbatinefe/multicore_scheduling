#include <stdio.h>
#include <stdlib.h> 
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <limits.h>
#include "constants.h"
#include "wbq.h"

extern int stop_threads;
extern int finished_jobs[NUM_CORES];
extern int flag_array[NUM_CORES];
extern WorkBalancerQueue** processor_queues;

// Thread function for each core simulator thread
void* processJobs(void* arg) {
    // initalize local variables
    ThreadArguments* my_arg = (ThreadArguments*) arg;
    WorkBalancerQueue* my_queue = my_arg -> q;
    int my_id = my_arg -> id;

    // Main loop, each iteration simulates the processor getting The stop_threads flag
    // is set by the main thread when it concludes all jobs are finished. The bookkeeping 
    // of finished jobs is done in executeJob's example implementation. a task from 
    // its or another processor's queue. After getting a task to execute the thread 
    // should call executeJob to simulate its execution. It is okay if the thread
    //  does busy waiting when its queue is empty and no other job is available
    // outside.
    while (!stop_threads) {
        // My Code
        Task *task = NULL;
        // We fetch the task here, it will return NULL if it is empty
        // This is called by owner of thread
        task = fetchTask(processor_queues[my_id]);
        
        /*
        * We have two options here
        * 1. If task is NULL, we can steal from others
        * 2. If task is not NULL, we can  directly execute it
        */

        if(task == NULL){
            // We update watermarks here considering average load of all queues
            updateQueueWatermarks(processor_queues[my_id], processor_queues, NUM_CORES);

            // We go to all cores's queue and check if we can steal
            for (int core = 0; core < NUM_CORES; core++){
                // We do not steal from ourselves AND 
                // We check watermark determined by updateQueueWatermarks (please see wbq.c)
                if(core != my_id && learnSize(processor_queues[core]) > processor_queues[core]->high_watermark){
                    // We fetch here, it will return NULL if selected queue is empty
                    // The reason selected queue can be empty is because our watermark level can go to 0
                    task = fetchTaskFromOthers(processor_queues[core]);
                    if(task != NULL) {
                        // We break if we stole it
                        break;
                    }
                }
            }
        }   

        // Now, end of two options: we have task from our queue or we stole it
        // We check if the duration is greater than 0, 
                // because maybe we stole a task that is already finished
        if(task && task->task_duration > 0){
            // We execute the job here
            executeJob(task, processor_queues[my_id], my_id);
            // If we have leftovers, we submit it to "OWNER" of the queue
            if(task->task_duration > 0){
                // TASK -> OWNER is the thread that task belongs
                // We simply get the owner id from there and submit it to that queue
                submitTask(processor_queues[task->owner->owner_id], task);
            }

        }    
        
    }
}

// Do any initialization of your shared variables here.
// For example initialization of your queues, any data structures 
// you will use for synchronization etc.
void initSharedVariables() {
    // My Code
    
    // We need NUM_CORES x queue so that each can work by themselves
    // Processor_queues will contain all queues globally.
    for (int c=0; c < NUM_CORES; c++){   
        // Allocate dummy node for starter that we keep the head
        // I emphasized this idea from MichaelScottQueue
        struct QueueNode *dummy = malloc(sizeof(struct QueueNode));
        dummy->task = NULL;
        dummy->nextPtr = NULL;
        dummy->prevPtr = NULL; 

        processor_queues[c]->head = dummy;
        processor_queues[c]->beforeTail = dummy;
        processor_queues[c]->tail = dummy;
        // Owner id is the id of the thread, it will be used to verify if task is owned by this thread
        processor_queues[c]->owner_id = c;

        // Remember black art -> if look too much -> high overhead
        // Remember black art -> if you do not look much -> load imbalances
        // This is just temporary value -> please check the algorithm on wbq.c
        processor_queues[c]->high_watermark = 4;
        processor_queues[c]->queue_size = 0; 

        pthread_mutex_init(&processor_queues[c]->head_lock, NULL); 
        pthread_mutex_init(&processor_queues[c]->tail_lock, NULL);
        pthread_mutex_init(&processor_queues[c]->size_lock, NULL);
        pthread_mutex_init(&processor_queues[c]->mark_lock, NULL); 
    }

}