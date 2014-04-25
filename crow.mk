

RAVEN_INC_DIRS := $(shell find $(RAVEN_DIR)/include -type d -not -path "*/.svn*")
RAVEN_INCLUDE  := $(foreach i, $(RAVEN_INC_DIRS), -I$(i))

libmesh_INCLUDE := $(RAVEN_INCLUDE) $(libmesh_INCLUDE)

RAVEN_LIB := $(RAVEN_DIR)/libRAVEN-$(METHOD).la

RAVEN_APP := $(RAVEN_DIR)/RAVEN-$(METHOD)

# source files
RAVEN_srcfiles    := $(shell find $(RAVEN_DIR)/src -name "*.C" -not -name main.C)
RAVEN_csrcfiles   := $(shell find $(RAVEN_DIR)/src -name "*.c")
RAVEN_fsrcfiles   := $(shell find $(RAVEN_DIR)/src -name "*.f")
RAVEN_f90srcfiles := $(shell find $(RAVEN_DIR)/src -name "*.f90")

# object files
RAVEN_objects := $(patsubst %.C, %.$(obj-suffix), $(RAVEN_srcfiles))
RAVEN_objects += $(patsubst %.c, %.$(obj-suffix), $(RAVEN_csrcfiles))
RAVEN_objects += $(patsubst %.f, %.$(obj-suffix), $(RAVEN_fsrcfiles))
RAVEN_objects += $(patsubst %.f90, %.$(obj-suffix), $(RAVEN_f90srcfiles))

# plugin files
RAVEN_plugfiles    := $(shell find $(RAVEN_DIR)/plugins/ -name "*.C" 2>/dev/null)
RAVEN_cplugfiles   := $(shell find $(RAVEN_DIR)/plugins/ -name "*.c" 2>/dev/null)
RAVEN_fplugfiles   := $(shell find $(RAVEN_DIR)/plugins/ -name "*.f" 2>/dev/null)
RAVEN_f90plugfiles := $(shell find $(RAVEN_DIR)/plugins/ -name "*.f90" 2>/dev/null)

# plugins
RAVEN_plugins := $(patsubst %.C, %-$(METHOD).plugin, $(RAVEN_plugfiles))
RAVEN_plugins += $(patsubst %.c, %-$(METHOD).plugin, $(RAVEN_cplugfiles))
RAVEN_plugins += $(patsubst %.f, %-$(METHOD).plugin, $(RAVEN_fplugfiles))
RAVEN_plugins += $(patsubst %.f90, %-$(METHOD).plugin, $(RAVEN_f90plugfiles))

# RAVEN main
RAVEN_main_src    := $(RAVEN_DIR)/src/main.C
RAVEN_app_objects := $(patsubst %.C, %.$(obj-suffix), $(RAVEN_main_src))

# dependency files
RAVEN_deps := $(patsubst %.C, %.$(obj-suffix).d, $(RAVEN_srcfiles)) \
              $(patsubst %.c, %.$(obj-suffix).d, $(RAVEN_csrcfiles)) \
              $(patsubst %.C, %.$(obj-suffix).d, $(RAVEN_main_src))

# clang static analyzer files
RAVEN_analyzer := $(patsubst %.C, %.plist.$(obj-suffix), $(RAVEN_srcfiles))

# If building shared libs, make the plugins a dependency, otherwise don't.
ifeq ($(libmesh_shared),yes)
  RAVEN_plugin_deps := $(RAVEN_plugins)
else
  RAVEN_plugin_deps :=
endif

all:: $(RAVEN_LIB)

$(RAVEN_LIB): $(RAVEN_objects) $(RAVEN_plugin_deps)
	@echo "Linking "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(libmesh_CXXFLAGS) -o $@ $(RAVEN_objects) $(libmesh_LIBS) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) -rpath $(RAVEN_DIR)
	@$(libmesh_LIBTOOL) --mode=install --quiet install -c $(RAVEN_LIB) $(RAVEN_DIR)

# Clang static analyzer
sa:: $(RAVEN_analyzer)

# include RAVEN dep files
-include $(RAVEN_deps)

# how to build RAVEN application
ifeq ($(APPLICATION_NAME),RAVEN)
all:: RAVEN

RAVEN_MODULES = $(RAVEN_DIR)/control_modules

$(RAVEN_DIR)/src/executioners/PythonControl.$(obj-suffix): $(RAVEN_DIR)/src/executioners/PythonControl.C
	@echo "Override PythonControl Compile"
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile --quiet \
          $(libmesh_CXX) $(libmesh_CPPFLAGS) $(libmesh_CXXFLAGS) $(PYTHON_INCLUDE) $(app_INCLUDES) -DRAVEN_MODULES='"$(RAVEN_MODULES)"' $(libmesh_INCLUDE) -MMD -MF $@.d -MT $@ -c $< -o $@


ifeq ($(UNAME),Darwin)
DISTRIBUTION_KLUDGE=$(RAVEN_LIB) 
else
DISTRIBUTION_KLUDGE=$(RAVEN_DIR)/src/distributions/distribution_1D.$(obj-suffix)  $(RAVEN_DIR)/src/distributions/distributionFunctions.$(obj-suffix) $(RAVEN_DIR)/src/base/RavenObject.$(obj-suffix) $(RAVEN_DIR)/src/distributions/distribution_base_ND.$(obj-suffix) $(RAVEN_DIR)/src/distributions/distribution.$(obj-suffix) $(RAVEN_DIR)/src/distributions/DistributionContainer.$(obj-suffix)
endif

$(RAVEN_DIR)/control_modules/_distribution1D.so : $(RAVEN_DIR)/control_modules/distribution1D.i \
                                                 $(RAVEN_DIR)/src/distributions/distribution_1D.C \
                                                 $(RAVEN_DIR)/src/distributions/distribution_base_ND.C \
                                                 $(RAVEN_DIR)/src/distributions/DistributionContainer.C \
                                                 $(RAVEN_DIR)/src/distributions/distributionFunctions.C \
                                                 $(RAVEN_DIR)/src/utilities/ND_Interpolation_Functions.C \
                                                 $(RAVEN_DIR)/src/utilities/microSphere.C \
                                                 $(RAVEN_DIR)/src/utilities/NDspline.C \
                                                 $(RAVEN_DIR)/src/utilities/inverseDistanceWeigthing.C \
                                                 $(RAVEN_DIR)/src/utilities/MDreader.C \
                                                 $(RAVEN_DIR)/src/distributions/distribution.C $(RAVEN_LIB)
# Swig
	swig -c++ -python $(SWIG_PY_FLAGS)  -I$(RAVEN_DIR)/include/distributions/  -Iinclude/base/ -Iinclude/utilities/ \
          $(RAVEN_MODULES)/distribution1D.i
# Compile
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile \
	$(libmesh_CXX) $(libmesh_CPPFLAGS) $(PYTHON_INCLUDE)\
         -I$(RAVEN_DIR)/include/distributions/ -I$(RAVEN_DIR)/include/utilities/ \
	 -c  $(RAVEN_MODULES)/distribution1D_wrap.cxx -o $(RAVEN_DIR)/control_modules/distribution1D_wrap.lo
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link \
	 $(libmesh_CXX) $(libmesh_CXXFLAGS) \
	-shared -o $(RAVEN_MODULES)/libdistribution1D.la $(PYTHON_LIB) $(RAVEN_MODULES)/distribution1D_wrap.lo $(DISTRIBUTION_KLUDGE) -rpath $(RAVEN_MODULES)
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=install install -c $(RAVEN_MODULES)/libdistribution1D.la  $(RAVEN_MODULES)/libdistribution1D.la 
	rm -f $(RAVEN_MODULES)/_distribution1D.so
	ln -s libdistribution1D.$(raven_shared_ext) $(RAVEN_MODULES)/_distribution1D.so



$(RAVEN_DIR)/control_modules/_raventools.so : $(RAVEN_DIR)/control_modules/raventools.i \
                                             $(RAVEN_DIR)/src/tools/batteries.C \
                                             $(RAVEN_DIR)/src/tools/DieselGeneratorBase.C \
                                             $(RAVEN_DIR)/src/tools/pumpCoastdown.C \
                                             $(RAVEN_DIR)/src/tools/decayHeat.C \
                                             $(RAVEN_DIR)/src/tools/powerGrid.C \
                                             $(RAVEN_DIR)/src/tools/RavenToolsContainer.C \
                                             $(RAVEN_DIR)/src/utilities/Interpolation_Functions.C $(RAVEN_LIB)
# Swig
	swig -c++ -python $(SWIG_PY_FLAGS) -I$(RAVEN_DIR)/../moose/include/base/  \
          -I$(RAVEN_DIR)/../moose/include/utils/ -I$(RAVEN_DIR)/include/tools/ \
          -I$(RAVEN_DIR)/include/utilities/ -I$(RAVEN_DIR)/include/base/ \
          $(RAVEN_MODULES)/raventools.i
#swig -c++ -python $(SWIG_PY_FLAGS) -I$(RAVEN_DIR)/include/tools/  -I$(RAVEN_DIR)/include/utilities/ $(RAVEN_DIR)/control_modules/raventools.i
# Compile
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile \
	$(libmesh_CXX) $(libmeh_CPPFLAGS) $(PYTHON_INCLUDE) $(app_INCLUDES)  $(libmesh_INCLUDE) \
	 -c  $(RAVEN_MODULES)/raventools_wrap.cxx -o $(RAVEN_DIR)/control_modules/raventools_wrap.lo
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link \
	 $(libmesh_CXX) $(libmesh_CXXFLAGS) \
	-o $(RAVEN_MODULES)/libraventools.la $(RAVEN_LIB) $(PYTHON_LIB) $(RAVEN_MODULES)/raventools_wrap.lo -rpath $(RAVEN_MODULES)
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=install install -c $(RAVEN_MODULES)/libraventools.la  $(RAVEN_MODULES)/libraventools.la 
	rm -f $(RAVEN_MODULES)/_raventools.so
	ln -s libraventools.$(raven_shared_ext) $(RAVEN_MODULES)/_raventools.so


RAVEN: $(RAVEN_APP) $(CONTROL_MODULES) $(PYTHON_MODULES)

$(RAVEN_APP): $(moose_LIB) $(elk_MODULES) $(r7_LIB) $(RAVEN_LIB) $(RAVEN_app_objects)
	@echo "Linking "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
          $(libmesh_CXX) $(libmesh_CXXFLAGS) -o $@ $(RAVEN_app_objects) $(RAVEN_LIB) $(r7_LIB) $(elk_MODULES) $(moose_LIB) $(libmesh_LIBS) $(libmesh_LDFLAGS) $(ADDITIONAL_LIBS) $(PYTHON_LIB) $(app_LIBS)

endif

delete_list := $(RAVEN_APP) $(RAVEN_LIB) $(RAVEN_DIR)/libRAVEN-$(METHOD).*

clean::
	@rm -f $(RAVEN_DIR)/control_modules/_distribution1D.so \
          $(RAVEN_DIR)/control_modules/_raventools.so \
          $(RAVEN_DIR)/control_modules/distribution1D_wrap.cxx \
          $(RAVEN_DIR)/control_modules/raventools_wrap.cxx \
          $(RAVEN_DIR)/control_modules/distribution1D.py \
          $(RAVEN_DIR)/control_modules/libdistribution1D.* \
          $(RAVEN_DIR)/control_modules/raventools.py \
          $(RAVEN_DIR)/control_modules/*.so* \
          $(RAVEN_DIR)/python_modules/*.so* \
          $(RAVEN_DIR)/python_modules/*_wrap.cxx \
          $(RAVEN_DIR)/python_modules/*py[23].py

clobber::
	@rm -f $(RAVEN_DIR)/control_modules/_distribution1D.so \
          $(RAVEN_DIR)/control_modules/distribution1D_wrap.cxx \
          $(RAVEN_DIR)/control_modules/distribution1D.py

cleanall:: 
	make -C $(RAVEN_DIR) clean 
