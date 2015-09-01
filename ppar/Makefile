BCC     = gcc
CFLAGS  = -Wall -ansi -I$(INC) -O3
LDFLAGS = -lm

INC = ./inc
SRC = ./src
OBJ = ./obj
BIN = ./bin

SEQ  = $(BIN)/sequential
PAR1 = $(BIN)/parallel-1
PAR2 = $(BIN)/parallel-2

all: sequential \
     parallel-1 \
     parallel-2 

# Create missing directories
DIRS = $(OBJ) $(BIN)
dirs: | $(DIRS)
$(DIRS):
	mkdir -p $(DIRS)

# Make sequential version
sequential: dirs $(SEQ)
$(SEQ): $(OBJ)/jacobi.o $(OBJ)/sequential.o
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ)/sequential.o: $(SRC)/sequential.c
	$(CC) -o $@ -c $< $(CFLAGS)

# Make parallel version 1
parallel-1: dirs $(PAR1)
$(PAR1): $(OBJ)/jacobi.o $(OBJ)/parallel-1.o
	mpicc -o $@ $^ $(LDFLAGS)

$(OBJ)/parallel-1.o: $(SRC)/parallel-1.c
	mpicc -o $@ -c $< $(CFLAGS)

# Make parallel version 2
parallel-2: dirs $(PAR2)
$(PAR2): $(OBJ)/jacobi.o $(OBJ)/parallel-2.o
	mpicc -o $@ $^ $(LDFLAGS)

$(OBJ)/parallel-2.o: $(SRC)/parallel-2.c
	mpicc -o $@ -c $< $(CFLAGS)

# Make Jacobi utils
$(OBJ)/jacobi.o: $(SRC)/jacobi.c
	$(CC) -o $@ -c $< $(CFLAGS)

clean:
	rm -fr $(OBJ)
	rm -fr $(BIN)
