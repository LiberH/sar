LC     = pdflatex
LFLAGS = -file-line-error \
	 -halt-on-error	  \
	 -output-directory $(OBJ) 

SRC = ./src
OBJ = ./obj
BIN = .

all: dirs target

DIRS = $(OBJ) $(BIN)
dirs: | $(DIRS)
$(DIRS):
	mkdir -p $(DIRS)

TARGET = $(BIN)/meeting.pdf
target: 
	$(LC) $(LFLAGS) $(SRC)/main
	@mv $(OBJ)/main.pdf $(TARGET)
	@touch $(TARGET) # To refresh the PDF viewer

clean:
	@rm -f $(TARGET)
	@rm -rf $(OBJ)
	@rm -f *~
	@rm -f $(SRC)/*~