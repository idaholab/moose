###############################################################################
######################### MOOSE Framework Makefile  ###########################
###############################################################################
#
# Required Make variables
# CURR_DIR	- current directory (DO NOT MODIFY THIS VARIABLE)
#
# Optional Environment variables
# MOOSE_DIR	- location of the MOOSE repository directory
###############################################################################
#all::

CURR_DIR	?= $(shell pwd)
MOOSE_DIR	?= $(shell dirname `pwd`)

#ROOT_DIR        ?= $(shell scripts/find_dep_apps.py ROOT)
#DEP_APPS	?= $(shell scripts/find_dep_apps.py MOOSE)

include build.mk
include moose.mk

###############################################################################
# Additional special case targets should be added here
