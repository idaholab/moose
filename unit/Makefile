###############################################################################
################### MOOSE Application Standard Makefile #######################
###############################################################################
#
# Required Environment variables
# LIBMESH_DIR	- location of the libMesh library
#
# Optional Environment variables
# CURR_DIR	- current directory (DO NOT MODIFY THIS VARIABLE)
# MOOSE_DIR	- location of the MOOSE framework
# ELK_DIR	- location of ELK (if enabled)
#
# Required Make variables
# APP_NAME	- the name of this application (all lower case)
# ENABLE_ELK 	- should be set to 'yes' to enable ELK
#
# Note: Make sure that there is no whitespace after the word 'yes' if enabling
# an application
###############################################################################
CURR_DIR	?= $(shell pwd)
MOOSE_DIR	?= $(shell pwd)/../moose
ENABLE_ELK 	:= no

MAKE_LIBRARY := no
APPLICATION_NAME := mooseunit

# Extra stuff for CPPUNIT
CPPUNIT_DIR 		?= $HOME/root/usr/local/packages/cppunit
ADDITIONAL_INCLUDES 	:= -I$(CPPUNIT_DIR)/include
ADDITIONAL_LIBS 	:= -L$(CPPUNIT_DIR)/lib -lcppunit 

# Include the MOOSE Export file
include $(MOOSE_DIR)/Makefile.export

###############################################################################
# Additional special case targets should be added here

#all::	mooseunit

#mooseunit: 
#	@$(MAKE) mooseunit-$(METHOD)

#elkunit: 
#	@$(MAKE) elkunit-$(METHOD) libmesh_CXXFLAGS="$(libmesh_CXXFLAGS) -DELK_TEST"

# Production rules:  how to make the target - depends on library configuration
#mooseunit-$(METHOD): $(objects) $(moose_LIB) $(mesh_library)
#	@echo "Linking "$@"..."
#	@$(libmesh_CXX) $(libmesh_CXXFLAGS) $(objects) -o $@ $(libmesh_LIBS) $(libmesh_LDFLAGS)

#elkunit-$(METHOD):  $(objects) $(moose_LIB) $(mesh_library)
#	@echo "Linking "$@"..."
#	@$(libmesh_CXX) $(libmesh_CXXFLAGS) $(objects) -o $@ $(libmesh_LIBS) $(libmesh_LDFLAGS)
