# A generic makfiatee listing the GNU targets 
SHELL = /bin/sh
CC = cc
LATEX = pdflatex
PDFLATEX = pdflatex -file-line-error --shell-escape
VIEW = open
BIBTEX = bibtex

BASE = lcio
default: $(BASE).pdf FORCE
all: $(BASE)

clean:	FORCE
	- rm -f *.o *.aux *.log *.toc *.bbl *.blg *.out *.nav *.snm *.toc *.vrb *.ptx *.dvi *.pdf *~

view: $(BASE).view

%.clean: FORCE 
	- rm $*.aux $*.log $*.out

%.pdf:	FORCE 
	- $(PDFLATEX) $*.tex
	- $(BIBTEX) $*
	- $(PDFLATEX) $*.tex

%.dvi:	%.tex
	- $(LATEX) $*
	- $(BIBTEX) $*
	- $(LATEX) $*

%.ps:	%.dvi
	dvips $*.dvi -o

#%.pdf:	%.ps
#	- ps2pdf $*.ps

%.view: %.pdf FORCE
	$(VIEW) $*.pdf
	-$*.pdf

distclean: clean
	- \rm -f *~ *.dvi 
	- \rm -rf auto

dist:   
install:
uninstall:
mostlyclean:
maintainer-clean:
TAGS:
info:
check:
	echo "Nothing done"


figs:
	gnuplot < gnuplot.cmds

FORCE:
