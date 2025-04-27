# Influential environment variables:
#
# NEML2_SRC_DIR: The path to the NEML2 source directory.
# NEML2_DIR:     The path to the installed NEML2.
#
# If NEML2_DIR is set, no lookup is done for the installed NEML2, and the
# specified directory is used.
#
# If NEML2_DIR is not set, by default we look for an installed NEML2 in
# the NEML2 submodule directory. If NEML2_SRC_DIR is set, we instead look for
# the installed NEML2 in NEML2_SRC_DIR/installed.

# Try out best to find the installed NEML2
ifeq ($(NEML2_DIR),)
ifneq ($(NEML2_SRC_DIR),)
NEML2_DIR := $(NEML2_SRC_DIR)/installed
else
NEML2_DIR := $(MOOSE_DIR)/framework/contrib/neml2/installed
endif
endif

# If the NEML2 directory does not exist, disable NEML2
ifeq ($(wildcard $(NEML2_DIR)/.),)
ENABLE_NEML2 = false
else
ENABLE_NEML2 = true
endif

# If MOOSE is not configured with libtorch, warn the user that NEML2 is disabled
ifeq ($(ENABLE_NEML2),true)
ifneq ($(ENABLE_LIBTORCH),true)
$(warning ******************************************************************************************)
$(warning Found a NEML2 installation at $(NEML2_DIR), but libtorch is not enabled. )
$(warning Disabling NEML2. To enable NEML2, please configure MOOSE following the instructions at )
$(warning https://mooseframework.inl.gov/getting_started/installation/install_libtorch.html)
$(warning ******************************************************************************************)
ENABLE_NEML2 = false
endif
endif

################################################################################
# ENABLE_NEML2 = true
################################################################################
ifeq ($(ENABLE_NEML2),true)

# Dynamic library suffix
DYLIB_SUFFIX := so
ifeq ($(shell uname -s),Darwin)
DYLIB_SUFFIX := dylib
endif

# NEML2 libraries are suffixed with the CMake build type
ifeq ($(METHOD),devel)
NEML2_SUFFIX := _RelWithDebInfo
else ifeq ($(METHOD),dbg)
NEML2_SUFFIX := _Debug
endif

# NEML2 directories and libraries
NEML2_INCLUDE    := $(NEML2_DIR)/include
# If we can ever consistently get NEML2 to install its libs in
# the same directory, we can get rid of this
ifneq ($(wildcard $(NEML2_DIR)/lib/.),)
NEML2_LIB_DIR 	 := $(NEML2_DIR)/lib
else ifneq ($(wildcard $(NEML2_DIR)/lib64/.),)
NEML2_LIB_DIR 	 := $(NEML2_DIR)/lib64
else
$(error Failed to find NEML2 libraries in $(NEML2_DIR)/lib or $(NEML2_DIR)/lib64)
endif
NEML2_LIBS       := neml2_base$(NEML2_SUFFIX) \
									  neml2_dispatcher$(NEML2_SUFFIX) \
										neml2_driver$(NEML2_SUFFIX) \
										neml2_jit$(NEML2_SUFFIX) \
										neml2_misc$(NEML2_SUFFIX) \
										neml2_model$(NEML2_SUFFIX) \
										neml2_solver$(NEML2_SUFFIX) \
										neml2_tensor$(NEML2_SUFFIX) \
										neml2_user_tensor$(NEML2_SUFFIX)
NEML2_LINK_FLAGS := $(addprefix -l,$(NEML2_LIBS))
NEML2_LIB_FILES  := $(addprefix $(NEML2_LIB_DIR)/lib,$(addsuffix .$(DYLIB_SUFFIX),$(NEML2_LIBS)))

# Compile flags for NEML2
neml2_INCLUDES    += $(addprefix -iquote,$(NEML2_INCLUDE))
neml2_CPPFLAGS    += -DNEML2_ENABLED
neml2_LDFLAGS     += $(NO_AS_NEEDED_FLAG) -Wl,-rpath,$(NEML2_LIB_DIR) -L$(NEML2_LIB_DIR) $(NEML2_LINK_FLAGS)
libmesh_CXXFLAGS += $(neml2_CPPFLAGS) $(neml2_INCLUDES)
libmesh_LIBS     += $(neml2_LDFLAGS)

endif
