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

###############################################################################
# Additional special case targets should be added here
