LC     = pdflatex
BC     = bibtex
LFLAGS = -file-line-error \
	 -halt-on-error	  \
	 -output-directory $(OBJ) 

SRC = ./src
OBJ = ./obj
BIN = .

all: dirs full

DIRS = $(OBJ) $(BIN)
dirs: | $(DIRS)
$(DIRS):
	@mkdir -p $(DIRS)

TARGET = $(BIN)/report.pdf
target: dirs
	$(LC) $(LFLAGS) $(SRC)/main
	@mv $(OBJ)/main.pdf $(TARGET)
	@touch $(TARGET) # To refresh the PDF viewer

bib:
	$(BC) $(OBJ)/main

full:
	@make target
	@make bib
	@make target
	@make target

clean:
	@rm -f $(TARGET)
	@rm -rf $(OBJ)
	@rm -f *~
	@rm -f $(SRC)/*~