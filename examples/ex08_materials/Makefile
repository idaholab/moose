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
MOOSE_DIR	?= $(shell pwd)/../../moose
ENABLE_ELK 	:= no

MAKE_LIBRARY := no
APPLICATION_NAME := ex08

# Include the MOOSE Export file
include $(MOOSE_DIR)/Makefile.export

###############################################################################
# Additional special case targets should be added here