###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Required Environment variables
# LIBMESH_DIR	- location of the libMesh library
#
# Required Make variables
# APPLICATION_NAME  - the name of this application (all lower case)
# MOOSE_DIR	- location of the MOOSE framework
# ELK_DIR	- location of ELK (if enabled)
#
# Optional Environment variables
# CURR_DIR	- current directory (DO NOT MODIFY THIS VARIABLE)
#
# Note: Make sure that there is no whitespace after the word 'yes' if enabling
# an application
###############################################################################
all::

CURR_DIR	?= $(shell pwd)
MOOSE_DIR	?= $(shell pwd)
ROOT_DIR        ?= $(shell scripts/find_dep_apps.py ROOT)
DEP_APPS	?= $(shell scripts/find_dep_apps.py MOOSE)

#Note: If you are a developer this will not work
# This line assumes that moose and libmesh are peer dirs
LIBMESH_DIR     ?= $(shell pwd)/../libmesh
include build.mk
include moose.mk

###############################################################################
# Additional special case targets should be added here



