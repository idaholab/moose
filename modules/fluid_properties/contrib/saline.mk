# source files
Saline_srcfiles    := $(shell find $(SALINE_SRC) -name "*.cc")

# object files
Saline_objects     := $(patsubst %.cc, %.$(obj-suffix), $(Saline_srcfiles))

# dependencies (C, C++ files only)
Saline_deps := $(patsubst %.cc, %.$(obj-suffix).d, $(Saline_srcfiles))

# clang static analyzer files
Saline_analyzer := $(patsubst %.cc, %.plist.$(obj-suffix), $(Saline_srcfiles))

app_LIBS       := $(SALINE_DIR)/lib/libSaline-$(METHOD).la $(app_LIBS)
app_LIBS_other := $(filter-out $(app_LIB),$(app_LIBS))

$(SALINE_DIR)/lib/libSaline-$(METHOD).la : $(Saline_objects)
	@$(shell mkdir -p $(SALINE_DIR)/lib)
	@echo "Linking Library "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
		$(libmesh_CXX) $(libmesh_CXXSHAREDFLAG) -o $@ $(Saline_objects) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) -rpath $(SALINE_DIR)/lib
	@$(libmesh_LIBTOOL) --mode=install --quiet install -c $@ $(SALINE_DIR)/lib

# Include dependencies (see note above)
-include $(Saline_deps)

ADDITIONAL_LIBS += $(SALINE_DIR)/lib/libSaline-$(METHOD).la

app_INCLUDES += -I$(SALINE_SRC) -I$(SALINE_SRC)/api -I$(SALINE_DIR)/include -I$(SALINE_SRC)/fortran
