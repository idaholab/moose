###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Required Environment variables
# LIBMESH_DIR	- location of the libMesh library
#
# Optional Environment variables
# CURR_DIR	- current directory (DO NOT MODIFY THIS VARIABLE)
# MOOSE_DIR	- location of the MOOSE framework
# ELK_DIR	- location of ELK (if enabled)
#
# Required Make variables
# APP_NAME	- the name of this application (all lower case)
# ENABLE_ELK 	- should be set to 'yes' to enable ELK
#
# Note: Make sure that there is no whitespace after the word 'yes' if enabling
# an application
###############################################################################
CURR_DIR	?= $(shell pwd)
MOOSE_DIR	?= $(shell pwd)
ENABLE_ELK 	:= no

MAKE_LIBRARY	:= yes
APPLICATION_NAME := moose

# Include the MOOSE Export file
include $(MOOSE_DIR)/Makefile.export.in
# Note: This export is in MOOSE therefore we can't include Makefile.export
#       here since it won't exist before the Makefile is executed.  If we
#       adopt a configure script the export file should be generated there
#       or in some sort of bootstrap script

###############################################################################
# Additional special case targets should be added here

all:: Makefile.export $(target)

Makefile.export: Makefile.export.in
	@echo "Creating Makefile.export"
	@cat Makefile.export.in > Makefile.export

clean::
	@rm -f Makefile.export
###############################################################################
