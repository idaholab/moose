###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Required Environment variables
# LIBMESH_DIR	- location of the libMesh library
#
# Optional Environment variables
# MOOSE_DIR	- location of the MOOSE framework
# ELK_DIR	- location of ELK (if enabled)
#
# Required Make variables
# APP_NAME	- the name of this application (all lower case)
# ENABLE_ELK 	- should be set to 'yes' to enable ELK

MOOSE_DIR 	?= $(shell pwd)/../moose
ENABLE_ELK 	:= no

# APPLICATION NAME (all lower case)
APPLICATION_NAME := moose_test

HYPRE_LINK_FLAGS=

# Include the MOOSE Export file
-include $(MOOSE_DIR)/Makefile.export


###############################################################################
# Additional special case targets should be added here
