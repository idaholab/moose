# Makefile for LaTeX files

# Original Makefile from http://www.math.psu.edu/elkin/math/497a/Makefile

# Please check http://www.acoustics.hut.fi/u/mairas/UltimateLatexMakefile
# for new versions.

# Copyright (c) 2005,2006 (in order of appearance):
#	Matti Airas <Matti.Airas@hut.fi>
# 	Rainer Jung
#	Antoine Chambert-Loir
#	Timo Kiravuo

# Permission is hereby granted, free of charge, to any person obtaining
# a copy of this software and associated documentation files (the
# "Software"), to deal in the Software without restriction, including
# without limitation the rights to use, copy, modify, merge, publish,
# distribute, sublicense, and/or sell copies of the Software, and to
# permit persons to whom the Software is furnished to do so, subject to
# the following conditions: 

# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software. 

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 

# $Id: Makefile,v 1.18 2006-06-19 10:58:11 mairas Exp $

#LATEX	= latex
LATEX	= pdflatex -shell-escape -interaction=nonstopmode
BIBTEX	= bibtex
MAKEINDEX = makeindex
XDVI	= xdvi -gamma 4
DVIPS	= dvips
DVIPDF  = dvipdft
L2H	= latex2html
GH	= gv

MUTE=@
#MUTE=
#SILENT=> /dev/null
SILENT=
VERYSILENT=1> /dev/null 2>/dev/null
#VERYSILENT=

RERUN = "(There were undefined references|Rerun to get (cross-references|the bars) right)"
RERUNBIB = "No file.*\.bbl|Citation.*undefined"
MAKEIDX = "^[^%]*\\makeindex"
MPRINT = "^[^%]*print"
USETHUMBS = "^[^%]*thumbpdf"

DATE=$(shell date +%Y-%m-%d)

COPY = if test -r $(<:%.tex=%.toc); then cp $(<:%.tex=%.toc) $(<:%.tex=%.toc.bak); fi 
RM = rm -f
OUTDATED = echo "EPS-file is out-of-date!" && false

# These are OK

SRC	:= $(shell egrep -l '^[^%]*\\begin\{document\}' *.tex)
TRG	= $(SRC:%.tex=%.dvi)
PSF	= $(SRC:%.tex=%.ps)
PDF	= $(SRC:%.tex=%.pdf)

# These are not

#BIBFILE := $(shell perl -ne '($$_)=/^[^%]*\\bibliography\{(.*?)\}/;@_=split /,/;foreach $$b (@_) {print "$$b.bib "}' $(SRC))
#DEP     := $(shell perl -ne '($$_)=/^[^%]*\\include\{(.*?)\}/;@_=split /,/;foreach $$t (@_) {print "$$t.tex "}' $(SRC))
#EPSPICS := $(shell perl -ne '@foo=/^[^%]*\\(includegraphics|psfig)(\[.*?\])?\{(.*?)\}/g;if (defined($$foo[2])) { if ($$foo[2] =~ /.eps$$/) { print "$$foo[2] "; } else { print "$$foo[2].eps "; }}' $(SRC) $(DEP))

define notify_latex
	echo "\nRunning LaTeX ..."
endef

define notify_bibtex
	echo "\nRunning BiBTeX ..."
endef


define run-latex
	$(MUTE)$(COPY);$(notify_latex);$(LATEX) $< $(SILENT)
	$(MUTE)egrep $(MAKEIDX) $< && ($(MAKEINDEX) $(<:%.tex=%);$(COPY);$(notify_latex);$(LATEX) $<) >/dev/null; true
	$(MUTE)egrep -c $(RERUNBIB) $(<:%.tex=%.log) $(SILENT) && ($(notify_bibtex);$(BIBTEX) $(<:%.tex=%) $(SILENT);$(COPY);$(notify_latex);$(LATEX) $< $(SILENT)) ; true
	$(MUTE)egrep $(RERUN) $(<:%.tex=%.log) && ($(COPY);$(notify_latex);$(LATEX) $<) >/dev/null; true
	$(MUTE)egrep $(RERUN) $(<:%.tex=%.log) && ($(COPY);$(notify_latex);$(LATEX) $<) >/dev/null; true
	$(MUTE)if cmp -s $(<:%.tex=%.toc) $(<:%.tex=%.toc.bak); then true ;else $(notify_latex);$(LATEX) $< $(SILENT); fi
	$(MUTE)$(RM) $(<:%.tex=%.toc.bak)
	# Display relevant warnings
	$(MUTE)egrep -i "(Reference|Citation).*undefined" $(<:%.tex=%.log) ; true
endef

define get_dependencies
	deps=`perl -ne '($$_)=/^[^%]*\\\(?:include|input|import)\{(.*?)\}(?:$$| )/;tr/}{//d;@_=split /,/;foreach $$t (@_) {print "$$t.tex "}' $<`
endef

define getbibs
	bibs=`perl -ne '($$_)=/^[^%]*\\\bibliography\{(.*?)\}/;@_=split /,/;foreach $$b (@_) { if (-e "$$b.bib") {print "$$b.bib "} }' $< $$deps`
endef

define geteps
	epses=`perl -ne '@foo=/^[^%]*\\\(includegraphics|psfig)(\[.*?\])?\{(.*?)\}/g;if (defined($$foo[2])) { if ($$foo[2] =~ /.eps$$/) { print "$$foo[2] "; } else { print "$$foo[2].eps "; }}' $< $$deps`
endef

define manconf
	mandeps=`if test -r $(basename $@).cnf ; then cat $(basename $@).cnf |tr -d '\n\r' ; fi`
endef

all 	: $(PDF)
#all 	: $(TRG)

.PHONY	: all show clean ps pdf showps veryclean

clean	:
	  $(MUTE)-rm -f $(TRG) $(PSF) $(PDF) $(TRG:%.dvi=%.aux) $(TRG:%.dvi=%.bbl) $(TRG:%.dvi=%.blg) $(TRG:%.dvi=%.log) $(TRG:%.dvi=%.out) $(TRG:%.dvi=%.idx) $(TRG:%.dvi=%.ilg) $(TRG:%.dvi=%.ind) $(TRG:%.dvi=%.toc) $(TRG:%.dvi=%.d)

veryclean	: clean
	  $(MUTE)-rm -f *.log *.aux *.dvi *.bbl *.blg *.ilg *.toc *.lof *.lot *.idx *.ind *.ps  *~

# This is a rule to generate a file of prerequisites for a given .tex file
%.d	: %.tex
	$(MUTE)$(get_dependencies) ; echo "Dependencies are: " $$deps ; $(getbibs) ; echo "Bibfiles are: " $$bibs ; echo "$*.pdf $@ : $< $$deps $$bibs " > $@ 
#	$(geteps) ; echo $$epses ; \
#	$(manconf) ; echo  $$mandeps  ;\
#	echo "$*.dvi $@ : $< $$deps $$bibs $$epses $$mandeps" > $@ 

include $(SRC:.tex=.d)

# $(DEP) $(EPSPICS) $(BIBFILE)
$(TRG)	: %.dvi : %.tex
	  $(MUTE)$(run-latex)

$(PSF)	: %.ps : %.dvi
	  $(MUTE)$(DVIPS) $< -o $@

#$(PDF)  : %.pdf : %.dvi
#	  $(MUTE)$(DVIPDF) -o $@ $<
# To use pdflatex, comment the two lines above and uncomment the lines below
$(PDF) : %.pdf : %.tex
	$(MUTE)$(run-latex)


show	: $(TRG)
	  $(MUTE)for i in $(TRG) ; do $(XDVI) $$i & done

showps	: $(PSF)
	  $(MUTE)for i in $(PSF) ; do $(GH) $$i & done

ps	: $(PSF) 

pdf	: $(PDF) 

# TODO: This probably needs fixing
html	: $(MUTE)$(DEP) $(EPSPICS)
	  $(MUTE)$(L2H) $(SRC)



######################################################################
# Define rules for EPS source files.

# These rules probably just cause unnecessary confusion, so commenting
# them away for the time being. -- mairas 2005-01-12

#%.eps: %.sxd
#	$(OUTDATED)
#%.eps: %.sda
#	$(OUTDATED)
#%.eps: %.png
#	$(OUTDATED)
#%.eps: %.sxc
#	$(OUTDATED)
#%.eps: %.xcf
#	$(OUTDATED)
#%.eps: %.zargo
#	$(OUTDATED)
#%.eps: %.m
#	$(MUTE)egrep -q $(MPRINT) $< && ($(OUTDATED))
