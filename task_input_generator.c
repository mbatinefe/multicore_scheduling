#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// This program enablues you to create a sample task input document
// for the simulator. The format of an individual task is ID-dur.
// Where ID is the task name and dur is the duration in ms.
// Each line denotes the initial queue of a separate processor. 
// The tasks in each line are separated by a space character ' '

// Note: To play around with different values, play with the calculation of ret.
// ret is calculated as rand() % (max - min + 1) + min
// For example to get jobs with a duration between [50, 400],
// we write ret = rand() % (400 - 50 + 1) + 50
// the % 100 line is there to round down last two digits. 

// Function to generate heavy tasks (between 2000 and 5000)
int generate_heavy_task() {
    int ret = rand() % (5000 - 2000 + 1) + 2000;
    ret -= ret % 100;
    return ret; 
}

// Function to generate light tasks (between 500 and 2000)
int generate_light_task() {
    int ret = rand() % (2000 - 500 + 1) + 500;
    ret -= ret % 100;
    return ret; 
}

// Function to generate tasks and write to a file
void generate_tasks(int n, int min_entries_per_line, int max_entries_per_line) {
    FILE *file = fopen("tasks.txt", "w");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Loop through the number of lines
    for (int i = 0; i < n; i++) {
        char task_prefix = 'A' + i; // Starting from 'A' for each line
        int num_entries = rand() % (max_entries_per_line - min_entries_per_line + 1) + min_entries_per_line;

        // Generate tasks for the current line
        for (int j = 1; j <= num_entries; j++) {
            int duration;
            if (rand() % 2 == 0) { // Randomly choose between heavy and light task
                duration = generate_heavy_task();
            } else {
                duration = generate_light_task();
            }
            fprintf(file, "%cTask%d-%d ", task_prefix, j, duration); // Write to file
        }
        fprintf(file, "\n"); // New line after each line of tasks
    }

    fclose(file);
}

int main() {
    srand(time(NULL)); // Seed for random number generation

    int n, min_entries_per_line, max_entries_per_line;

    // Input the number of lines and entry limits
    printf("Enter the number of lines: ");
    scanf("%d", &n);
    printf("Enter the minimum entries per line: ");
    scanf("%d", &min_entries_per_line);
    printf("Enter the maximum entries per line: ");
    scanf("%d", &max_entries_per_line);

    // Generate tasks and write to the file
    generate_tasks(n, min_entries_per_line, max_entries_per_line);

    printf("Tasks generated and written to tasks.txt\n");
    
    return 0;
}

// This code was written by our AI overlords.
