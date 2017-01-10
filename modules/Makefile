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

################################## MODULES ####################################
ALL_MODULES       := yes
INCLUDE_COMBINED  := no
include modules.mk
###############################################################################

# Build the Module executable
APPLICATION_DIR    := $(MOOSE_DIR)/modules/combined
APPLICATION_NAME   := combined
BUILD_EXEC         := yes
# Note: There are no applications that depend on moose_test so the test_up and up
# targets are not very useful here.  Instead we will test up like this is MOOSE
DEP_APPS           ?= $(shell $(FRAMEWORK_DIR)/scripts/find_dep_apps.py $(APPLICATION_NAME))
include            $(FRAMEWORK_DIR)/app.mk

###############################################################################
# Additional special case targets should be added here

# This target will build the individual executables in each module
# and run their respective tests.
MODULE_DIRS    := $(shell find $(MOOSE_DIR)/modules -maxdepth 1 -mindepth 1 -type d -not -name doc)
MOOSE_COLOR        ?= true
ifneq ($(MOOSE_COLOR),true)
  COLOR_STRING = '--no-color'
endif

test_subs:
	@echo ======================================================
	@echo Testing the following modules:
	@for app in $(MODULE_DIRS); do echo \ $$app; done
	@echo ======================================================
	@echo
	@(ret_val=0; \
	for app in $(MODULE_DIRS); \
	do \
		echo ====== Testing in $${app} ====== ; \
		cd $${app} && $(MAKE) && ./run_tests $(COLOR_STRING) -j $(MOOSE_JOBS) $(MOOSE_LOAD) -t || ret_val=1; \
	done; \
	exit $$ret_val;)
