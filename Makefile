CC=gcc
DEPS = constants.h wbq.h

sim: sim_methods.c simulator.c wbq.c $(DEPS)
	$(CC) -o sim sim_methods.c simulator.c wbq.c

generator: task_input_generator.c
	$(CC) -o generator task_input_generator.c
