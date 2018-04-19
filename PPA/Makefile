CC      = gcc
CFLAGS  = -Wall -ansi -I$(INC) -O3
LDFLAGS = -lm

CUDA_CC      = nvcc
CUDA_CFLAGS  = --generate-code arch=compute_20,code=sm_20 -O3 --compiler-options -Wall,-ansi,-I$(INC),-O3
CUDA_LDFLAGS = -L/usr/local/cuda/lib64/ -lcuda -lcudart

INC = ./inc
SRC = ./src
OBJ = ./obj
BIN = ./bin

all: main

main: main.o
	$(CUDA_CC) $^ $(LDFLAGS) $(CUDA_LDFLAGS)

%.o: %.cu
	$(CUDA_CC) $(CUDA_CFLAGS) -o $@ -c $<

clean:
	rm -f ./a.out
	rm -f ./*.o
	@rm -f ./*~