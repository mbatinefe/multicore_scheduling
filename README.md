# Multicore Task Scheduling

A C implementation of a work-stealing scheduler for multicore task execution with cache-aware optimizations.

## Overview

This project implements a work-balancing queue system for distributing and executing tasks across multiple CPU cores. It features:

- Dynamic task stealing between cores to balance workload
- Cache-aware execution with warmup factors
- Lock-free queue operations for better performance
- Adaptive watermark-based load balancing

## Files

- `simulator.c` - Main scheduling logic and thread management
- `wbq.c/h` - Work balancing queue implementation 
- `sim_methods.c` - Task execution and simulation methods
- `constants.h` - Configuration constants
- `task_input_generator.c` - Utility to generate sample task files
- `sample_tasks*.txt` - Example task input files

## Building

Use make to build the project:

```sh
make sim      # Build the simulator
make generator # Build the task generator
```

## Usage
1. Generate task input file (optional):

```sh
./generator
```

2. Run the simulator with a task file:
```sh
./sim sample_tasks1.txt
```

## Task Format

Each line represents tasks for one processor:
TaskID-Duration TaskID-Duration ...

Example:
ATask1-180 ATask2-390 ATask3-940
BTask1-670 BTask2-550 BTask3-510

Format Details:
- TaskID: Unique alphanumeric identifier
- Duration: Task runtime in milliseconds
- Tasks are space-separated
- Each line represents a different core's tasks

## Configuration

Key parameters in `constants.h`:

| Parameter | Default | Description |
|-----------|---------|-------------|
| NUM_CORES | 4 | Number of processor cores to simulate |
| CYCLE | 100 | CPU cycle duration in milliseconds |
| CACHE_FACTOR | 0.2 | Cache warmup increment per task |
| MAX_CACHE_FACTOR | 2.0 | Maximum cache warmup multiplier |
| QUEUE_SIZE | 1024 | Maximum tasks per work queue |
| STEAL_THRESHOLD | 0.3 | Load threshold for task stealing |

Example configuration:

```c
#define NUM_CORES 4
#define CYCLE 100
#define CACHE_FACTOR 0.2
#define MAX_CACHE_FACTOR 2.0
#define QUEUE_SIZE 1024
#define STEAL_THRESHOLD 0.3
```

To modify:

1. Open constants.h
2. Update desired parameters
3. Rebuild project with make