CC      = gcc
CFLAGS  = -ansi -Wall -Werror -pedantic -I$(INC)
#CFLAGS += -DDEBUG
LDFLAGS = -lnsl -pthread

INC = ./inc
SRC = ./src
OBJ = ./obj
BIN = ./bin


all: dirs target

DIRS = $(OBJ) $(BIN)
dirs: | $(DIRS)
$(DIRS):
	mkdir -p $(DIRS)

TRGT =	$(BIN)/losh
OBJS =	$(OBJ)/utils.o		\
	$(OBJ)/xdr.o		\
	$(OBJ)/information.o	\
	$(OBJ)/localisation.o	\
	$(OBJ)/transfert.o	\
	$(OBJ)/slave.o		\
	$(OBJ)/ihm.o            \
	$(OBJ)/master.o		\
	$(OBJ)/main.o
target: $(TRGT)
$(TRGT): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	rm -f $(TRGT)
	rm -f $(OBJ)/*
