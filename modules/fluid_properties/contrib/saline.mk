# source files
Saline_srcfiles    := $(shell find $(SALINE_SRC) -name "*.cc")

# object files
Saline_objects     := $(patsubst %.cc, %.$(obj-suffix), $(Saline_srcfiles))

Saline_LIB := $(SALINE_DIR)/libSaline-$(METHOD).la

# dependencies (C, C++ files only)
Saline_deps := $(patsubst %.cc, %.$(obj-suffix).d, $(Saline_srcfiles))

# clang static analyzer files
Saline_analyzer := $(patsubst %.cc, %.plist.$(obj-suffix), $(Saline_srcfiles))

$(Saline_LIB) : $(Saline_objects)
	@$(shell mkdir -p $(SALINE_DIR)/lib)
	@echo "Linking Library "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
		$(libmesh_CXX) $(libmesh_CXXSHAREDFLAG) -o $@ $(Saline_objects) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) -rpath $(SALINE_DIR)
	@$(libmesh_LIBTOOL) --mode=install --quiet install -c $(Saline_LIB) $(SALINE_DIR)

# Include dependencies (see note above)
-include $(Saline_deps)

ADDITIONAL_INCLUDES += -I$(SALINE_SRC) -I$(SALINE_DIR)/include -I$(SALINE_SRC)/fortran
ADDITIONAL_LIBS += -L$(SALINE_DIR) -lSaline-$(METHOD)
ADDITIONAL_DEPEND_LIBS += $(Saline_LIB)
