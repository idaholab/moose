MOOSE_DIR ?= $(shell pwd)/../../..
FRAMEWORK_DIR ?= $(MOOSE_DIR)/framework

all: SoftwareRequirements.pdf framework_requirements_traceability.pdf

SoftwareRequirements.pdf: SoftwareRequirements.tex INLreport.cls DOEbwlogo.pdf inl-left_black.pdf
	pdflatex SoftwareRequirements.tex

framework_requirements_traceability.pdf:
	"$(MOOSE_DIR)/scripts/traceability_matrix.py" --application "$(FRAMEWORK_DIR)"

clean:
	@rm -f *.log *.aux
	@rm -f SoftwareRequirements.pdf framework_requirements_traceability.pdf
