CROW_INC_DIRS := $(shell find $(CROW_DIR)/include -type d -not -path "*/.svn*")
CROW_INCLUDE  := $(foreach i, $(CROW_INC_DIRS), -I$(i))

libmesh_INCLUDE := $(CROW_INCLUDE) $(libmesh_INCLUDE)

CROW_LIB := $(CROW_DIR)/libCROW-$(METHOD).la

# source files
CROW_srcfiles    := $(shell find $(CROW_DIR)/src -name "*.C" -not -name main.C)
CROW_csrcfiles   := $(shell find $(CROW_DIR)/src -name "*.c")
CROW_fsrcfiles   := $(shell find $(CROW_DIR)/src -name "*.f")
CROW_f90srcfiles := $(shell find $(CROW_DIR)/src -name "*.f90")

# object files
CROW_objects := $(patsubst %.C, %.$(obj-suffix), $(CROW_srcfiles))
CROW_objects += $(patsubst %.c, %.$(obj-suffix), $(CROW_csrcfiles))
CROW_objects += $(patsubst %.f, %.$(obj-suffix), $(CROW_fsrcfiles))
CROW_objects += $(patsubst %.f90, %.$(obj-suffix), $(CROW_f90srcfiles))

# plugin files
CROW_plugfiles    := $(shell find $(CROW_DIR)/plugins/ -name "*.C" 2>/dev/null)
CROW_cplugfiles   := $(shell find $(CROW_DIR)/plugins/ -name "*.c" 2>/dev/null)
CROW_fplugfiles   := $(shell find $(CROW_DIR)/plugins/ -name "*.f" 2>/dev/null)
CROW_f90plugfiles := $(shell find $(CROW_DIR)/plugins/ -name "*.f90" 2>/dev/null)

# plugins
CROW_plugins := $(patsubst %.C, %-$(METHOD).plugin, $(CROW_plugfiles))
CROW_plugins += $(patsubst %.c, %-$(METHOD).plugin, $(CROW_cplugfiles))
CROW_plugins += $(patsubst %.f, %-$(METHOD).plugin, $(CROW_fplugfiles))
CROW_plugins += $(patsubst %.f90, %-$(METHOD).plugin, $(CROW_f90plugfiles))

# dependency files
 CROW_deps := $(patsubst %.C, %.$(obj-suffix).d, $(CROW_srcfiles)) \
	      $(patsubst %.c, %.$(obj-suffix).d, $(CROW_csrcfiles))

# clang static analyzer files
CROW_analyzer := $(patsubst %.C, %.plist.$(obj-suffix), $(CROW_srcfiles))

# If building shared libs, make the plugins a dependency, otherwise don't.
ifeq ($(libmesh_shared),yes)
  CROW_plugin_deps := $(CROW_plugins)
else
  CROW_plugin_deps :=
endif

all:: $(CROW_LIB)
$(CROW_LIB): $(CROW_objects) $(CROW_plugin_deps)
	@echo "Linking "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(libmesh_CXXFLAGS) -o $@ $(CROW_objects) $(libmesh_LIBS) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) -rpath $(CROW_DIR)
	@$(libmesh_LIBTOOL) --mode=install --quiet install -c $(CROW_LIB) $(CROW_DIR)

# Clang static analyzer
sa:: $(CROW_analyzer)

# include CROW dep files
-include $(CROW_deps)

all:: CROW

CROW_MODULES = $(CROW_DIR)/control_modules

ifeq ($(UNAME),Darwin)
DISTRIBUTION_KLUDGE=$(CROW_LIB)
else
DISTRIBUTION_KLUDGE=$(CROW_DIR)/src/distributions/distribution_1D.$(obj-suffix)  $(CROW_DIR)/src/distributions/distributionFunctions.$(obj-suffix) $(CROW_DIR)/src/distributions/distributionNDBase.$(obj-suffix) $(CROW_DIR)/src/distributions/distributionNDNormal.$(obj-suffix) $(CROW_DIR)/src/distributions/distribution.$(obj-suffix) $(CROW_DIR)/src/distributions/DistributionContainer.$(obj-suffix)
endif

$(CROW_DIR)/control_modules/_distribution1D.so : $(CROW_DIR)/control_modules/distribution1D.i \
						 $(CROW_DIR)/src/distributions/distribution_1D.C \
						 $(CROW_DIR)/src/distributions/distributionNDBase.C \
						 $(CROW_DIR)/src/distributions/distributionNDNormal.C \
						 $(CROW_DIR)/src/distributions/DistributionContainer.C \
						 $(CROW_DIR)/src/distributions/distributionFunctions.C \
						 $(CROW_DIR)/src/utilities/ND_Interpolation_Functions.C \
						 $(CROW_DIR)/src/utilities/microSphere.C \
						 $(CROW_DIR)/src/utilities/NDspline.C \
						 $(CROW_DIR)/src/utilities/inverseDistanceWeigthing.C \
						 $(CROW_DIR)/src/utilities/MDreader.C \
						 $(CROW_DIR)/src/distributions/distribution.C $(CROW_LIB)
# Swig
	swig -c++ -python $(SWIG_PY_FLAGS)  -I$(CROW_DIR)/include/distributions/  -Iinclude/base/ -Iinclude/utilities/ \
	  $(CROW_MODULES)/distribution1D.i
# Compile
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile \
	$(libmesh_CXX) $(libmesh_CPPFLAGS) $(PYTHON_INCLUDE)\
	 -I$(CROW_DIR)/include/distributions/ -I$(CROW_DIR)/include/utilities/ -I$(CROW_LIB_INCLUDE_DIR) -std=c++11 \
	 -c  $(CROW_MODULES)/distribution1D_wrap.cxx -o $(CROW_DIR)/control_modules/distribution1D_wrap.lo
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link \
	 $(libmesh_CXX) $(libmesh_CXXFLAGS) \
	-shared -o $(CROW_MODULES)/libdistribution1D.la $(PYTHON_LIB) $(CROW_MODULES)/distribution1D_wrap.lo $(DISTRIBUTION_KLUDGE) -rpath $(CROW_MODULES)
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=install install -c $(CROW_MODULES)/libdistribution1D.la  $(CROW_MODULES)/libdistribution1D.la
	rm -f $(CROW_MODULES)/_distribution1D.so
	ln -s libdistribution1D.$(crow_shared_ext) $(CROW_MODULES)/_distribution1D.so



$(CROW_DIR)/control_modules/_crowtools.so : $(CROW_DIR)/control_modules/crowtools.i \
					     $(CROW_DIR)/src/tools/batteries.C \
					     $(CROW_DIR)/src/tools/DieselGeneratorBase.C \
					     $(CROW_DIR)/src/tools/pumpCoastdown.C \
					     $(CROW_DIR)/src/tools/decayHeat.C \
					     $(CROW_DIR)/src/tools/powerGrid.C \
					     $(CROW_DIR)/src/tools/CrowToolsContainer.C \
					     $(CROW_DIR)/src/utilities/Interpolation_Functions.C $(CROW_LIB)
# Swig
	swig -c++ -python $(SWIG_PY_FLAGS) -I$(CROW_DIR)/../moose/include/base/  \
	  -I$(CROW_DIR)/../moose/include/utils/ -I$(CROW_DIR)/include/tools/ \
	  -I$(CROW_DIR)/include/utilities/ -I$(CROW_DIR)/include/base/ \
	  $(CROW_MODULES)/crowtools.i
#swig -c++ -python $(SWIG_PY_FLAGS) -I$(CROW_DIR)/include/tools/  -I$(CROW_DIR)/include/utilities/ $(CROW_DIR)/control_modules/crowtools.i
# Compile
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=compile \
	$(libmesh_CXX) $(libmesh_CPPFLAGS) $(CXXFLAGS) $(PYTHON_INCLUDE) $(app_INCLUDES)  $(libmesh_INCLUDE) -std=c++11 \
	 -c  $(CROW_MODULES)/crowtools_wrap.cxx -o $(CROW_DIR)/control_modules/crowtools_wrap.lo
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link \
	 $(libmesh_CXX) $(libmesh_CXXFLAGS) \
	-o $(CROW_MODULES)/libcrowtools.la $(CROW_LIB) $(PYTHON_LIB) $(CROW_MODULES)/crowtools_wrap.lo -rpath $(CROW_MODULES)
	$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=install install -c $(CROW_MODULES)/libcrowtools.la  $(CROW_MODULES)/libcrowtools.la
	rm -f $(CROW_MODULES)/_crowtools.so
	ln -s libcrowtools.$(crow_shared_ext) $(CROW_MODULES)/_crowtools.so


CROW: $(CONTROL_MODULES) $(PYTHON_MODULES)

delete_list := $(CROW_APP) $(CROW_LIB) $(CROW_DIR)/libCROW-$(METHOD).*

.PHONY : crow_clean

clean :: crow_clean

crow_clean :
	@rm -f $(CROW_DIR)/control_modules/_distribution1D.so \
	  $(CROW_DIR)/control_modules/_crowtools.so \
	  $(CROW_DIR)/control_modules/distribution1D_wrap.cxx \
	  $(CROW_DIR)/control_modules/crowtools_wrap.cxx \
	  $(CROW_DIR)/control_modules/distribution1D.py \
	  $(CROW_DIR)/control_modules/libdistribution1D.* \
	  $(CROW_DIR)/control_modules/crowtools.py \
	  $(CROW_DIR)/control_modules/*.so* \
	  $(CROW_DIR)/control_modules/*.dylib* \
          $(CROW_DIR)/control_modules/*.lo \
          $(CROW_DIR)/control_modules/*.la \
          $(CROW_DIR)/control_modules/*.pyc \
	  $(CROW_DIR)/crow_modules/*.so* \
	  $(CROW_DIR)/crow_modules/*.dylib* \
	  $(CROW_DIR)/crow_modules/*_wrap.cxx \
	  $(CROW_DIR)/crow_modules/*_wrap.cpp \
	  $(CROW_DIR)/crow_modules/*py[23].py \
	  $(CROW_DIR)/crow_modules/*.pyc \
	  $(CROW_LIB) $(CROW_DIR)/libCROW-$(METHOD).*
	@rm -Rf $(CROW_DIR)/install/ $(CROW_DIR)/build/
	@rm -Rf $(CROW_DIR)/control_modules/.libs/ $(CROW_DIR)/.libs/
	@rm -Rf $(CROW_DIR)/src/distributions/.libs/
	@rm -Rf $(CROW_DIR)/src/tools/.libs/
	@rm -Rf $(CROW_DIR)/src/utilities/.libs/
	@rm -Rf $(CROW_DIR)/src/distributions/*.lo \
	  $(CROW_DIR)/src/tools/*.lo \
	  $(CROW_DIR)/src/utilities/*.lo
	@rm -Rf $(CROW_DIR)/src/distributions/*.d \
	  $(CROW_DIR)/src/tools/*.d \
	  $(CROW_DIR)/src/utilities/*.d

clobber::
	@rm -f $(CROW_DIR)/control_modules/_distribution1D.so \
	  $(CROW_DIR)/control_modules/distribution1D_wrap.cxx \
	  $(CROW_DIR)/control_modules/distribution1D.py

cleanall::
	make -C $(CROW_DIR) clean
