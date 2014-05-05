###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Optional Environment variables
# MOOSE_DIR     - Root directory of the MOOSE project
# FRAMEWORK_DIR - Location of the MOOSE framework
#
###############################################################################
EXAMPLE_DIR        ?= $(pwd)
MOOSE_DIR          ?= $(shell dirname $(EXAMPLE_DIR))
FRAMEWORK_DIR      ?= $(MOOSE_DIR)/framework
###############################################################################

TEST := test_ignore

# framework
include $(FRAMEWORK_DIR)/build.mk
include $(FRAMEWORK_DIR)/moose.mk

## Moose Examples
all:: examples

dirlist := $(wildcard ex*/)

#
# Build the example programs
examples: $(dirlist)

$(dirlist): $(MOOSE_LIBS)
	@$(MAKE) -C $@ $(MAKECMDGOALS) TEST=test_ignore || exit 1


.PHONY: clean clobber distclean link run .depend test subdirs $(dirlist)

#
# Clean up the example programs
#clean::#
#	@for dir in $(dirlist); do $(MAKE) -C "$${dir}" $(MAKECMDGOALS) TEST=test_ignore || exit 1; done

#
# Really clean up the example programs
#clobber::
#	@for dir in $(dirlist); do $(MAKE) -C "$${dir}" $(MAKECMDGOALS) TEST=test_ignore || exit 1; done

# Make the example program directories look
# like a clean distribution
#distclean::
#	@for dir in $(dirlist); do $(MAKE) -C "$${dir}" $(MAKECMDGOALS) TEST=test_ignore || exit 1; done

# Link the example programs
link:: $(wildcard examples/ex*/*.C)
	@for dir in $(dirlist); do $(MAKE) -C "$${dir}" all TEST=test_ignore || exit 1; done

#
# Rebuild the dependencies for the examples
.depend: $(wildcard examples/ex*/*.C)
	@for dir in $(dirlist); do $(MAKE) -C "$${dir}" $(MAKECMDGOALS) TEST=test_ignore || exit 1; done

test:
	@for dir in $(dirlist); do $(MAKE) -C "$${dir}" $(MAKECMDGOALS) TEST=test_ignore || exit 1; done


