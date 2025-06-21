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
else ifeq ($(METHOD),oprof)
NEML2_SUFFIX := _Profiling
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
ifeq ($(HAVE_NO_AS_NEEDED),yes)
  neml2_LDFLAGS += $(NO_AS_NEEDED_FLAG)
endif
neml2_LDFLAGS     += -Wl,-rpath,$(NEML2_LIB_DIR) -L$(NEML2_LIB_DIR) $(NEML2_LINK_FLAGS)
libmesh_CXXFLAGS  += $(neml2_CPPFLAGS)
libmesh_INCLUDE   += $(neml2_INCLUDES)
libmesh_LIBS      += $(neml2_LDFLAGS)

endif

################################################################################
# Helper target to check if NEML2 is enabled and show relevant flags
################################################################################
.PHONY: check_neml2
check_neml2:
ifeq ($(ENABLE_NEML2),true)
	@echo "NEML2 is enabled"
	@echo "NEML2 directory: $(NEML2_DIR)"
	@echo "NEML2 cppflags: $(neml2_CPPFLAGS)"
	@echo "NEML2 includes: $(neml2_INCLUDES)"
	@echo "NEML2 ldflags: $(neml2_LDFLAGS)"
else
	@echo "NEML2 is disabled"
endif
