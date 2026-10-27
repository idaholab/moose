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

TEST := test_ignore

# framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

dirlist := $(wildcard ex*/)

.PHONY: examples test .depend $(dirlist)

## Moose Examples
all: examples

# Build the example programs
examples: $(dirlist)

$(dirlist): $(moose_LIBS)
	@$(MAKE) -C $@ $(MAKECMDGOALS) TEST=test_ignore || exit 1

# Rebuild the dependencies for the examples
.depend: $(wildcard examples/ex*/*.C)
	@for dir in $(dirlist); do $(MAKE) -C "$${dir}" $(MAKECMDGOALS) TEST=test_ignore || exit 1; done

test:
	@for dir in $(dirlist); do $(MAKE) -C "$${dir}" $(MAKECMDGOALS) TEST=test_ignore || exit 1; done
