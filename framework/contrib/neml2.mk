################################################################################
# ENABLE_NEML2 = true
################################################################################
ifeq ($(ENABLE_NEML2),true)

# NEML2 libraries and pkgconfig are suffixed with the CMake build type
ifeq ($(METHOD),devel)
NEML2_SUFFIX := _RelWithDebInfo
else ifeq ($(METHOD),oprof)
NEML2_SUFFIX := _Profiling
else ifeq ($(METHOD),dbg)
NEML2_SUFFIX := _Debug
endif

# NEML2 pkgconfig file
NEML2_PC := $(NEML2_DIR)/share/pkgconfig/neml2$(NEML2_SUFFIX).pc
ifeq ($(wildcard $(NEML2_PC)),)
$(error NEML2 pkgconfig file not found: $(NEML2_PC))
endif

# NEML2 flags
comma := ,
neml2_CPPFLAGS += -DNEML2_ENABLED
neml2_INCLUDES += $(shell pkg-config --cflags $(NEML2_PC))
neml2_LDFLAGS  += $(foreach libdir,$(shell pkg-config --libs-only-L $(NEML2_PC)),$(patsubst -L%,-Wl$(comma)-rpath$(comma)%,$(libdir)))
neml2_LIBS     += $(shell pkg-config --libs $(NEML2_PC))

# Append to libmesh flags
libmesh_CXXFLAGS += $(neml2_CPPFLAGS) # I think MOOSE Makefiles tend to abuse libmesh_CXXFLAGS for preprocessor flags
libmesh_INCLUDE  += $(neml2_INCLUDES)
libmesh_LDFLAGS  += $(neml2_LDFLAGS)
libmesh_LIBS     += $(neml2_LIBS)

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
	@echo "NEML2 libs: $(neml2_LIBS)"
else
	@echo "NEML2 is disabled"
endif
