LC     = pdflatex
LFLAGS = -file-line-error \
	 -halt-on-error	  \
	 -output-directory $(OBJ) 

SRC = ./src
OBJ = ./obj
BIN = .

all: dirs target pres

DIRS = $(OBJ) $(BIN)
dirs: | $(DIRS)
$(DIRS):
	mkdir -p $(DIRS)

PRES = $(BIN)/PSAR_pres.pdf
TRGT = $(BIN)/PSAR_rapport.pdf
target: 
	$(LC) $(LFLAGS) $(SRC)/Main.tex
	mv $(OBJ)/Main.pdf $(TRGT)
	@touch $(TRGT) # To refresh the PDF viewer

pres: 
	$(LC) $(LFLAGS) $(SRC)/Presentation.tex
	mv $(OBJ)/Presentation.pdf $(PRES)
	@touch $(PRES) # To refresh the PDF viewer

clean:
	rm -f $(TRGT)
	rm -f $(PRES)
	rm -f $(OBJ)/*
