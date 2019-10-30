NAME = INLreport_example

TEXINPUTS = ".:..:"
TEX_DIRS = $(shell sed "s/:/ /g" <<< $(TEXINPUTS))
TEX_FILES = $(shell find $(TEX_DIRS) -type f -name '*.tex' -o -name '*.cls')
BIB_FILES = references1.bib references2.bib

all: $(NAME).pdf

quick: $(TEX_FILES) $(BIB_FILES)
	@TEXINPUTS=$(TEXINPUTS) pdflatex -jobname=$(NAME) $(NAME)

$(NAME).pdf: $(TEX_FILES) $(BIB_FILES)
	@TEXINPUTS=$(TEXINPUTS) pdflatex -jobname=$(NAME) $(NAME)
	@bibtex $(NAME)
	@TEXINPUTS=$(TEXINPUTS) pdflatex -jobname=$(NAME) $(NAME)
	@TEXINPUTS=$(TEXINPUTS) pdflatex -jobname=$(NAME) $(NAME)

.PHONY: clean

clean:
	@rm -f *~ *.aux *.bbl *.blg *.log *.out *.toc *.lot *.lof *.xwm $(NAME).pdf
