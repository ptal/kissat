CC=gcc
CFLAGS=-W -ansi -Wall -Werror -O3 -std=c18
LD=gcc

BUILD_DIR=build
SRC=src

KISSAT_INCLUDES=-I../src/
KISSAT_LIB=-L../build/ -lkissat


all: simplifier service.o

clean:
	rm $(BUILD_DIR)/*

simplifier: $(SRC)/simplifier.c
	$(CC) $(CFLAGS) $(KISSAT_INCLUDES) $(SRC)/simplifier.c -o $(BUILD_DIR)/simplifier $(KISSAT_LIB) -lm

# -D_DEFAULT_SOURCE is used to have access to DT_REG.
# It could be avoided by using `stat`.
service.o: $(SRC)/service.c
	$(CC) $(CFLAGS) -c -D_DEFAULT_SOURCE $(SRC)/service.c -o $(BUILD_DIR)/service.o

.PHONY: all clean
