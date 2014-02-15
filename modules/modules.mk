###############################################################################
############################ COMMON MODULES ###################################
###############################################################################
ifeq ($(ALL_MODULES),yes)
	FOOBAR                    := yes

	libmesh_CXXFLAGS          += -DALL_MODULES
endif

###############################################################################
########################## MODULE REGISTRATION ################################
###############################################################################
#
# New Module Step 4.
#
# ifeq ($(MODULENAME),yes)
# APPLICATION_DIR    := $(MOOSE_DIR)/modules/modulename
# APPLICATION_NAME   := modulename
# include $(FRAMEWORK_DIR)/app.mk
# libmesh_CXXFLAGS   += -DMODULENAME
# endif
#
###############################################################################

ifeq ($(FOOBAR),yes)
APPLICATION_DIR    := $(MOOSE_DIR)/modules/foobar
APPLICATION_NAME   := foobar
include $(FRAMEWORK_DIR)/app.mk
libmesh_CXXFLAGS   += -DFOOBAR
endif
