# source files
Thermochimica_srcfiles    := $(shell find $(THERMOCHIMICA_SRC) -name "*.C")
Thermochimica_f90modfiles := $(shell find $(THERMOCHIMICA_SRC) -name "Module*.f90")
Thermochimica_f90srcfiles := $(shell find $(THERMOCHIMICA_SRC) -name "*.f90" -not -name "Module*")

# object files (from C)
Thermochimica_objects     := $(patsubst %.C, %.$(obj-suffix), $(Thermochimica_srcfiles))

# the C files depend on MooseConfig.h
ifeq ($(MOOSE_HEADER_SYMLINKS),true)
$(Thermochimica_objects): $(moose_config_symlink)
else
$(Thermochimica_objects): $(moose_config)
endif

# remaining object files (Fortran)
Thermochimica_objects     += $(patsubst %.f90, %.$(obj-suffix), $(Thermochimica_f90modfiles))
Thermochimica_objects     += $(patsubst %.f90, %.$(obj-suffix), $(Thermochimica_f90srcfiles))

# dependencies (C, C++ files only)
Thermochimica_deps := $(patsubst %.C, %.$(obj-suffix).d, $(Thermochimica_srcfiles)) \
                      $(patsubst %.c, %.$(obj-suffix).d, $(Thermochimica_csrcfiles))

# clang static analyzer files
Thermochimica_analyzer := $(patsubst %.C, %.plist.$(obj-suffix), $(Thermochimica_srcfiles))

app_LIBS       := $(THERMOCHIMICA_DIR)/lib/libThermochimica-$(METHOD).la $(app_LIBS)
app_LIBS_other := $(filter-out $(app_LIB),$(app_LIBS))

$(THERMOCHIMICA_DIR)/lib/libThermochimica-$(METHOD).la : $(Thermochimica_objects)
	@$(shell mkdir -p $(THERMOCHIMICA_DIR)/lib)
	@echo "Linking Library "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(libmesh_CXXSHAREDFLAG) -o $@ $(Thermochimica_objects) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) -rpath $(THERMOCHIMICA_DIR)/lib
	@$(libmesh_LIBTOOL) --mode=install --quiet install -c $@ $(THERMOCHIMICA_DIR)/lib

# Include dependencies (see note above)
-include $(Thermochimica_deps)

ADDITIONAL_LIBS += $(THERMOCHIMICA_DIR)/lib/libThermochimica-$(METHOD).la

app_INCLUDES += -I$(THERMOCHIMICA_SRC) -I$(THERMOCHIMICA_SRC)/api -I$(THERMOCHIMICA_SRC)/module

# F90 module dependency rules
$(Thermochimica_f90srcfiles): $(patsubst %.f90, %.$(obj-suffix), $(Thermochimica_f90modfiles))
