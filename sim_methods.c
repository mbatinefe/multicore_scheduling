#include <unistd.h>
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include "wbq.h"
#include "constants.h"

// This is a sample file we will use to call your API and test.
// executeJob is the function you will call to simulate task execution
// The main function is responsible for parsing the inputs, calling your
// thread and initialization functions in simulator.c, and then monitoring
// execution while your threads run.

//shared vars
int stop_threads = 0;
int finished_jobs[NUM_CORES];
WorkBalancerQueue** processor_queues;
// Timing
clock_t program_start, program_end;
double total_execution_time;

// Simulate task execution
void executeJob(Task* task, WorkBalancerQueue* my_queue, int my_id ) {
    // Update task's affinity and owner thread 
    // if it was recently acquired by another thread
    if (task -> owner != my_queue) {
        task -> owner = my_queue;
        task -> cache_warmed_up = 1.0;
    } 

    // If the next execution finishes the task, set its remaining time to 0
    // Notify the main thread that a job was finished by updating finished_jobs.
    // Else, update cache factor and duration accordingly.
    if (task -> task_duration - (CYCLE * task -> cache_warmed_up) <= 0) {
        task -> task_duration = 0;
        printf("Processor %d: Finished task %s\n", my_id, task -> task_id);
        finished_jobs[my_id]++;
    } else {
        task -> task_duration -= CYCLE * task -> cache_warmed_up;
        printf("Processor %d: Executed task %s, it has %d ms remaining\n", my_id, task -> task_id, task->task_duration);
        if (task -> cache_warmed_up < MAX_CACHE_FACTOR ) task -> cache_warmed_up += CACHE_FACTOR;
    }

    // Sleep for one simulate CPU cycle
    usleep(CYCLE * 1000);
}

// Check if sufficient number of jobs were finished.
int all_jobs_finished(int registered_jobs) {
    int sum = 0;
    for (int i = 0; i < NUM_CORES; i++) {
        sum += finished_jobs[i];
    }
    return sum >= registered_jobs;
}

int main(int argc, char* argv[]) {
    program_start = clock();

    // Parse command line arguments
    if (argc != 2) {
        fprintf(stderr, "Incorrect call, usage: %s <filename>\n", argv[0]);
        return 1;
    }
    
    char* filename = argv[1];

    // Try to open the input file
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        printf("Couldn't open file , terminating. . .\n");
        return -1;
    }


    // Initialize shared variables, call the student's function as well.
    processor_queues = malloc(NUM_CORES * sizeof(WorkBalancerQueue*));
    for (int i = 0; i < NUM_CORES; i++) {
        processor_queues[i] = malloc(sizeof(WorkBalancerQueue));
        finished_jobs[i] = 0;
    }
    initSharedVariables();

    printf("Initialized %d processor_queues\n", NUM_CORES);

    
    // Parse input file
    char line[1024];
    int processor_index = 0;
    int registered_jobs = 0;
    int max_runtime = INT_MIN;
    while (fgets(line, sizeof(line), file)) { 
        int core_runtime = 0;
        int num;
        char *token = strtok(line, " ");  
        while (token != NULL) {
            char string_part[1024];
            int num;

            if (sscanf(token, "%[^-]-%d", string_part, &num) == 2) {
                Task* my_task = malloc(sizeof(Task));
                my_task -> task_id = strdup(string_part);
                my_task -> task_duration = num;
                my_task -> cache_warmed_up = 1.0;
                my_task -> owner = processor_queues[processor_index];
                core_runtime += num;
                submitTask(processor_queues[processor_index], my_task);
                registered_jobs++;
            }

            token = strtok(NULL, " "); 
        }
        if (core_runtime > max_runtime) max_runtime = core_runtime;
        processor_index++;
    }

    fclose(file);
    printf("Read file, starting multithreaded execution\n");
    // To print the initial state of cores after tasks are distributed
    // printf("Initial state of cores:\n");
    // for (int i = 0; i < NUM_CORES; i++) {
    //     printf("Core %d: ", i);
    //     print_queue(processor_queues[i]);
    // }
    // printf("---------------------------------------------\n");

    // Start threads
    pthread_t processor_ids[NUM_CORES];
    for (int i = 0; i < NUM_CORES; i++) {
        ThreadArguments* arg = malloc(sizeof(ThreadArguments));
        arg -> q = processor_queues[i];
        arg -> id = i;
        int rc = pthread_create(&processor_ids[i], NULL, &processJobs, arg);
        if (rc) {
            printf("Error creating thread, terminating. . . ");
            return -1;
        }
    }

    // Sleep until tasks are finished
    // If you want to debug, you can uncomment this print block
    // to see a snapshot of the state of your queues, implement a print_queue method first
    while (!all_jobs_finished(registered_jobs)) {
        // for (int i = 0; i < NUM_CORES; i++) {
        //     printf("Core %d: ", i);
        //     print_queue(processor_queues[i]);
        // }
        usleep(2000 * 1000);
    }

    stop_threads = 1;

    printf("All tasks finished, joining threads\n");

    for (int i = 0; i < NUM_CORES; i++) {
        pthread_join(processor_ids[i], NULL);
    }

    program_end = clock();
    total_execution_time = ((double) (program_end - program_start)) / CLOCKS_PER_SEC;
    //printf("\nTotal program execution time: %.3f seconds\n", total_execution_time);
   
    return 0;
}
