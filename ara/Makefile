LC     = pdflatex
LFLAGS = -file-line-error \
	 -halt-on-error	  \
	 -output-directory $(OBJ) 

SRC = ./src
OBJ = ./obj
BIN = .

all: dirs report

DIRS = $(OBJ) $(BIN)
dirs: | $(DIRS)
$(DIRS):
	mkdir -p $(DIRS)

TRGT = $(BIN)/Report.pdf
report: 
	$(LC) $(LFLAGS) $(SRC)/Main.tex
	mv $(OBJ)/Main.pdf $(TRGT)
	@touch $(TRGT) # To refresh the PDF viewer

clean:
	rm -f $(TRGT)
	rm -rf $(OBJ)