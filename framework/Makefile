###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Required Environment variables
# LIBMESH_DIR	- location of the libMesh library
#
# Optional Environment variables
# MOOSE_DIR	- location of the MOOSE framework
# ELK_DIR	- location of ELK (if enabled)
#
# Required Make variables
# APP_NAME	- the name of this application (all lower case)
# ENABLE_ELK 	- should be set to 'yes' to enable ELK

MOOSE_DIR	?= $(shell pwd)
ENABLE_ELK 	:= no

# APPLICATION NAME (all lower case)
APPLICATION_NAME := libmoose

# Include the MOOSE Export file
-include $(MOOSE_DIR)/Makefile.export.in
# Note: This export is in MOOSE therefore we can't include Makefile.export
#       here since it won't exist before the Makefile is executed.  If we
#       adopt a configure script the export file should be generated there
#       or in some sort of bootstrap script

###############################################################################
# Additional special case targets should be added here

target := $(MOOSE_DIR)/$(APPLICATION_NAME)-$(METHOD)$(static_libext)

ifeq ($(enable-shared),yes)
  target := $(MOOSE_DIR)/$(APPLICATION_NAME)-$(METHOD)$(shared_libext)
endif

all:: Makefile.export $(target)

Makefile.export: Makefile.export.in
	@echo "Creating Makefile.export"
	@cat Makefile.export.in >> Makefile.export

ifeq ($(enable-shared),yes)
# Build dynamic library
$(target): $(objects)
	@echo "Linking "$@
	@$(libmesh_CC) $(libmesh_CXXSHAREDFLAG) -o $@ $(objects) $(libmesh_LDFLAGS)
else
# Build static library
ifeq ($(findstring darwin,$(hostos)),darwin)
$(target): $(objects)
	@echo "Linking "$@
	@libtool -static -o $@ $(objects)
else
$(target): $(objects)
	@echo "Linking "$@
	@$(AR) rv $@ $(objects)
endif
endif

clean::
	@rm -f Makefile.export
###############################################################################

