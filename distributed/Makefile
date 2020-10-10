CC=gcc
# -D_DEFAULT_SOURCE is used to have access to functionalities such as DT_REG and strdup.
CFLAGS=-W -ansi -Wall -Werror -O3 -std=c18 -D_DEFAULT_SOURCE
LD=gcc

BUILD_DIR=build
SRC=src

KISSAT_INCLUDES=-I../src/
KISSAT_LIB=-L../build/ -lkissat

CORE=$(SRC)/service.c $(SRC)/network.c
COREOBJ=$(BUILD_DIR)/service.o $(BUILD_DIR)/network.o
SIMPLIFIER_SRC=$(SRC)/simplifier.c
SPLIT_SRC=$(SRC)/split.c
SOLVER_SRC=$(SRC)/solver.c


all: service.o network.o simplifier split solver

clean:
	rm $(BUILD_DIR)/*

service.o: $(SRC)/service.c
	$(CC) $(CFLAGS) -c $(SRC)/service.c -o $(BUILD_DIR)/service.o

network.o: $(SRC)/network.c
	$(CC) $(CFLAGS) -c $(SRC)/network.c -o $(BUILD_DIR)/network.o -lzmq

simplifier: $(SRC)/simplifier.c
	$(CC) $(CFLAGS) $(KISSAT_INCLUDES) $(SIMPLIFIER_SRC) -o $(BUILD_DIR)/simplifier $(COREOBJ) $(KISSAT_LIB) -lm -lzmq

solver: $(COREOBJ) $(SRC)/solver.c
	$(CC) $(CFLAGS) $(KISSAT_INCLUDES) $(SOLVER_SRC) -o $(BUILD_DIR)/solver $(COREOBJ) $(KISSAT_LIB) -lm -lzmq

split: $(COREOBJ) $(SRC)/split.c
	$(CC) $(CFLAGS) $(KISSAT_INCLUDES) $(SPLIT_SRC) -o $(BUILD_DIR)/split $(COREOBJ) $(KISSAT_LIB) -lm -lzmq

.PHONY: all clean
