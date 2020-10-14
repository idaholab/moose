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

################################## MODULES ####################################
ALL_MODULES       := yes
INCLUDE_COMBINED  := no
USE_TEST_LIBS     := yes
include modules.mk
###############################################################################

# Build the Module executable
APPLICATION_DIR    := $(MOOSE_DIR)/modules/combined
APPLICATION_NAME   := combined
BUILD_EXEC         := yes
GEN_REVISION       := no
include            $(FRAMEWORK_DIR)/app.mk

###############################################################################
# Additional special case targets should be added here

# This target will build the individual executables in each module
# and run their respective tests.
MODULE_DIRS    := $(shell find $(MOOSE_DIR)/modules -maxdepth 1 -mindepth 1 -type d -not -name doc -not -name module_loader -not -name combined)
MOOSE_COLOR        ?= true
ifneq ($(MOOSE_COLOR),true)
  COLOR_STRING = '--no-color'
endif

.PHONY: builds $(MODULE_DIRS)
builds: all $(MODULE_DIRS)

$(MODULE_DIRS): $(app_LIBS)
	@$(MAKE) -C $@

test_subs: all builds
	@echo ======================================================
	@echo Testing the following modules:
	@for app in $(MODULE_DIRS); do echo \ $$app; done
	@echo ======================================================
	@echo
	@(ret_val=0; \
	for app in $(MODULE_DIRS); \
	do \
		echo ====== Testing in $${app} ====== ; \
		cd $${app} && ./run_tests $(COLOR_STRING) -j $(MOOSE_JOBS) $(MOOSE_LOAD) -t || ret_val=1; \
	done; \
	exit $$ret_val;)

MOOSE_DOCS_DIR ?= $(MOOSE_DIR)/site
test_doc: all builds
	@echo ======================================================
	@echo Testing MooseDocs for the following modules:
	@for app in $(MODULE_DIRS); do echo \ $$app; done
	@echo ======================================================
	@echo
	@(ret_val=0; \
	for app in $(MODULE_DIRS); \
	do \
		echo ====== Testing MooseDocs in $${app} ====== ; \
		cd $${app}/doc && ./moosedocs.py build --destination=$(MOOSE_DOCS_DIR) || ret_val=1; \
	done; \
	exit $$ret_val;)
