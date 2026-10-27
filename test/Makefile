###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Optional Environment variables
# MOOSE_DIR     - Root directory of the MOOSE project
# FRAMEWORK_DIR - Location of the MOOSE framework
#
###############################################################################
MOOSE_DIR          ?= $(shell dirname "$$PWD")
FRAMEWORK_DIR      ?= $(MOOSE_DIR)/framework
ifneq ($(words $(MOOSE_DIR)),1)
  $(error MOOSE_DIR contains whitespace: '$(MOOSE_DIR)'. MOOSE Makefiles do not support paths with whitespace; move the checkout or set MOOSE_DIR to a path without whitespace)
endif
ifneq ($(words $(FRAMEWORK_DIR)),1)
  $(error FRAMEWORK_DIR contains whitespace: '$(FRAMEWORK_DIR)'. MOOSE Makefiles do not support paths with whitespace; move the checkout or set FRAMEWORK_DIR to a path without whitespace)
endif
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
