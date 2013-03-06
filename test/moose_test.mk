moose_test_INC_DIRS := $(shell find $(MOOSE_TEST_DIR)/include -type d -not -path "*/.svn*")
moose_test_INCLUDE  := $(foreach i, $(moose_test_INC_DIRS), -I$(i))

libmesh_INCLUDE := $(moose_test_INCLUDE) $(libmesh_INCLUDE)

moose_test_LIB := $(MOOSE_TEST_DIR)/libmoose_test-$(METHOD).la

moose_test_APP := $(MOOSE_TEST_DIR)/moose_test-$(METHOD)

# source files
moose_test_srcfiles    := $(shell find $(MOOSE_TEST_DIR)/src -name "*.C" -not -name main.C)
moose_test_csrcfiles   := $(shell find $(MOOSE_TEST_DIR)/src -name "*.c")
moose_test_fsrcfiles   := $(shell find $(MOOSE_TEST_DIR)/src -name "*.f")
moose_test_f90srcfiles := $(shell find $(MOOSE_TEST_DIR)/src -name "*.f90")

# object files
moose_test_objects := $(patsubst %.C, %.$(obj-suffix), $(moose_test_srcfiles))
moose_test_objects += $(patsubst %.c, %.$(obj-suffix), $(moose_test_csrcfiles))
moose_test_objects += $(patsubst %.f, %.$(obj-suffix), $(moose_test_fsrcfiles))
moose_test_objects += $(patsubst %.f90, %.$(obj-suffix), $(moose_test_f90srcfiles))

# plugin files
moose_test_plugfiles    := $(shell find $(MOOSE_TEST_DIR)/plugins/ -name "*.C" 2>/dev/null)
moose_test_cplugfiles   := $(shell find $(MOOSE_TEST_DIR)/plugins/ -name "*.c" 2>/dev/null)
moose_test_fplugfiles   := $(shell find $(MOOSE_TEST_DIR)/plugins/ -name "*.f" 2>/dev/null)
moose_test_f90plugfiles := $(shell find $(MOOSE_TEST_DIR)/plugins/ -name "*.f90" 2>/dev/null)

# plugins
moose_test_plugins := $(patsubst %.C, %-$(METHOD).plugin, $(moose_test_plugfiles))
moose_test_plugins += $(patsubst %.c, %-$(METHOD).plugin, $(moose_test_cplugfiles))
moose_test_plugins += $(patsubst %.f, %-$(METHOD).plugin, $(moose_test_fplugfiles))
moose_test_plugins += $(patsubst %.f90, %-$(METHOD).plugin, $(moose_test_f90plugfiles))

# moose_test main
moose_test_main_src    := $(MOOSE_TEST_DIR)/src/main.C
moose_test_app_objects := $(patsubst %.C, %.$(obj-suffix), $(moose_test_main_src))

# dependency files
moose_test_deps := $(patsubst %.C, %.$(obj-suffix).d, $(moose_test_srcfiles)) \
              $(patsubst %.c, %.$(obj-suffix).d, $(moose_test_csrcfiles)) \
              $(patsubst %.C, %.$(obj-suffix).d, $(moose_test_main_src))

# If building shared libs, make the plugins a dependency, otherwise don't.
ifeq ($(libmesh_shared),yes)
  moose_test_plugin_deps := $(moose_test_plugins)
else
  moose_test_plugin_deps :=
endif

all:: $(moose_test_LIB)

$(moose_test_LIB): $(moose_test_objects) $(moose_test_plugin_deps)
	@echo "Linking "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(libmesh_CXXFLAGS) -o $@ $(moose_test_objects) $(libmesh_LIBS) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) -rpath $(MOOSE_TEST_DIR)
	@$(libmesh_LIBTOOL) --mode=install --quiet install -c $(moose_test_LIB) $(MOOSE_TEST_DIR)

# include MOOSE_TEST dep files
-include $(moose_test_deps)

# how to build MOOSE_TEST application
ifeq ($(APPLICATION_NAME),moose_test)
all:: moose_test

moose_test: $(moose_test_APP)

$(moose_test_APP): $(moose_LIB) $(elk_MODULES) $(moose_test_LIB) $(moose_test_app_objects)
	@echo "Linking "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
          $(libmesh_CXX) $(libmesh_CXXFLAGS) -o $@ $(moose_test_app_objects) $(moose_test_LIB) $(elk_MODULES) $(moose_LIB) $(libmesh_LIBS) $(libmesh_LDFLAGS) $(ADDITIONAL_LIBS)

endif

#
# Maintenance
#
delete_list := $(moose_test_APP) $(moose_test_LIB) $(MOOSE_TEST_DIR)/libmoose_test-$(METHOD).*

cleanall:: 
	make -C $(MOOSE_TEST_DIR) clean 

###############################################################################
# Additional special case targets should be added here
