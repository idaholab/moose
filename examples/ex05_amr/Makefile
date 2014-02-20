###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Optional Environment variables
# MOOSE_DIR     - Root directory of the MOOSE project 
# FRAMEWORK_DIR - Location of the MOOSE framework
#
###############################################################################
MOOSE_DIR          ?= $(shell dirname `pwd`)
FRAMEWORK_DIR      ?= $(MOOSE_DIR)/framework
###############################################################################

# framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

APPLICATION_NAME := ex05

# dep apps
APPLICATION_DIR    := $(shell pwd)
APPLICATION_NAME   := ex05
BUILD_EXEC         := yes
DEP_APPS           := $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))
include            $(FRAMEWORK_DIR)/app.mk
include            ../test.mk

# Include dependency files for this example
ex_srcfiles := $(shell find $(APPLICATION_DIR) -name "*.C")
ex_deps     := $(patsubst %.C, %.$(obj-suffix).d, $(ex_srcfiles))
-include $(ex_deps)

###############################################################################
# Additional special case targets should be added here

test: $(APPLICATION_NAME)-$(METHOD)
	$(call TEST_exodiff,ex05.i,out.e-s003)
