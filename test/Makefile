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
ADDITIONAL_CPPFLAGS += -Wall -Wextra
###############################################################################

# framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

# dep apps
APPLICATION_DIR    := $(MOOSE_DIR)/test
APPLICATION_NAME   := moose_test
BUILD_EXEC         := yes
BUILD_TEST_OBJECTS_LIB := no
GEN_REVISION       := no
include            $(FRAMEWORK_DIR)/app.mk

###############################################################################
# Additional special case targets should be added here

MOOSE_DOCS_DIR ?= $(MOOSE_DIR)/site
test_doc: all
	@echo ======================================================
	@echo Testing MooseDocs for the framework and MooseTestApp
	@echo ======================================================
	@(ret_val=0; \
	cd doc && ./moosedocs.py build --destination=$(MOOSE_DOCS_DIR) || ret_val=1; \
	exit $$ret_val;)
