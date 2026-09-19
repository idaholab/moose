################################################################################
# ENABLE_NEML2 = true
################################################################################
ifeq ($(ENABLE_NEML2),true)

# MOOSE links the non-editable neml2 PyPI wheel, which ships a single unsuffixed (Release) C++
# build -- libneml2 / libneml2_eager and neml2.pc. Every MOOSE method (opt/oprof/dbg/devel) links
# this Release build: a dbg/oprof MOOSE still has full debug/profiling info for its own code, and
# the neml2 dependency is optimized.

# NEML2 pkgconfig file
NEML2_PC := $(NEML2_DIR)/share/pkgconfig/neml2.pc
ifeq ($(wildcard $(NEML2_PC)),)
$(error NEML2 pkgconfig file not found: $(NEML2_PC))
endif

# NEML2 flags
comma := ,
neml2_CPPFLAGS += -DNEML2_ENABLED

# When MOOSE manages torch separately (ENABLE_LIBTORCH=true), filter torch flags out of
# NEML2's bundled pkg-config. Otherwise libtool deduplicates the -l flags keeping the last
# occurrence (embedded deep in the .la chain), pushing torch after petsc.
NEML2_TORCH_PC := $(NEML2_DIR)/share/pkgconfig/neml2-torch.pc
ifneq ($(wildcard $(NEML2_TORCH_PC)),)
neml2_torch_INCLUDES := $(shell pkg-config --cflags $(NEML2_TORCH_PC))
neml2_torch_LDIRS    := $(shell pkg-config --libs-only-L $(NEML2_TORCH_PC))
neml2_torch_LIBS     := $(shell pkg-config --libs $(NEML2_TORCH_PC))
endif

neml2_INCLUDES += $(filter-out $(neml2_torch_INCLUDES),$(shell PKG_CONFIG_PATH=$(NEML2_DIR)/share/pkgconfig:${PKG_CONFIG_PATH} pkg-config --cflags $(NEML2_PC)))
neml2_LDFLAGS  += $(foreach libdir,$(filter-out $(neml2_torch_LDIRS),$(shell PKG_CONFIG_PATH=$(NEML2_DIR)/share/pkgconfig:${PKG_CONFIG_PATH} pkg-config --libs-only-L $(NEML2_PC))),$(patsubst -L%,-Wl$(comma)-rpath$(comma)%,$(libdir)))
neml2_LIBS     += $(filter-out $(neml2_torch_LIBS),$(shell PKG_CONFIG_PATH=$(NEML2_DIR)/share/pkgconfig:${PKG_CONFIG_PATH} pkg-config --libs $(NEML2_PC)))

# NEML2 v3 ships two C++ libraries: libneml2 (the aoti runtime, listed by neml2.pc above)
# and libneml2_eager (the embedded-Python eager runtime, used by MOOSE for the no-compile
# path). The eager lib is not listed in any .pc file and depends on the aoti lib, so add it
# explicitly and ahead of -lneml2 on the link line.
neml2_LIBS     := -lneml2_eager $(neml2_LIBS)

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
