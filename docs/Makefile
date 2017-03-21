srcdir=.

DOCS_DIR = $(srcdir)

EXAMPLES = $(DOCS_DIR)/example1.C \
           $(DOCS_DIR)/example2.C

all: user-manual.pdf

user-manual.pdf: $(DOCS_DIR)/user-manual.tex $(EXAMPLES)
	@pdflatex $(DOCS_DIR)/user-manual
#	@bibtex $(DOCS_DIR)/user-manual
	@pdflatex $(DOCS_DIR)/user-manual
	@pdflatex $(DOCS_DIR)/user-manual
	@rm -f *.log *.aux *.toc
	@echo "Generated: user-manual.pdf"

clean:
	@rm -f *.log *.aux *.toc *.bbl *.blg

distclean: clean
	@rm -f *.pdf 

check:
	aspell -c -t user-manual.tex

