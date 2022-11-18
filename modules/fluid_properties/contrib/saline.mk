# source files
Saline_srcfiles    := $(shell find $(SALINE_SRC) -name "*.cc")
#$(shell find $(SALINE_SRC) -name "*.cpp")
#Saline_f90modfiles := $(shell find $(SALINE_SRC) -name "*.f90")
#Saline_f90srcfiles := $(shell find $(SALINE_SRC) -name "*.f90" -not -name "Module*")

# object files
Saline_objects     := $(patsubst %.cc, %.$(obj-suffix), $(Saline_srcfiles))
#Saline_objects     += $(patsubst %.cpp, %.$(obj-suffix), $(Saline_srcfiles))
#Saline_objects     += $(patsubst %.f90, %.$(obj-suffix), $(Saline_f90modfiles))
#Saline_objects     += $(patsubst %.f90, %.$(obj-suffix), $(Thermochimica_f90srcfiles))

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

# F90 module dependency rules
#$(Saline_f90srcfiles): $(patsubst %.f90, %.$(obj-suffix), $(Saline_f90modfiles))
