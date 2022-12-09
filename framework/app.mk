# This file contains common MOOSE application settings
# Note: MOOSE applications are assumed to reside in peer directories relative to MOOSE and its modules.
#       This can be overridden by using the MOOSE_DIR environment variable.

# This variable is used to determine whether a C++ header revision file is generated for use
# in your application. You can turn it on/off by changing it in your application Makefile.
GEN_REVISION ?= yes

# list of application-wide excluded source files
excluded_srcfiles :=

#
# Save off parameters for possible app.mk recursion
#
STACK ?= stack
STACK := $(STACK).X
$APPLICATION_DIR$(STACK) := $(APPLICATION_DIR)
$APPLICATION_NAME$(STACK) := $(APPLICATION_NAME)
$DEPEND_MODULES$(STACK) := $(DEPEND_MODULES)
$GEN_REVISION$(STACK) := $(GEN_REVISION)
$BUILD_EXEC$(STACK) := $(BUILD_EXEC)

-include $(APPLICATION_DIR)/$(APPLICATION_NAME).mk

# install target related stuff
share_install_dir := $(share_dir)/$(APPLICATION_NAME)
docs_install_dir := $(share_install_dir)/doc

#
# Restore parameters
#
APPLICATION_DIR := $($APPLICATION_DIR$(STACK))
APPLICATION_NAME := $($APPLICATION_NAME$(STACK))
DEPEND_MODULES := $($DEPEND_MODULES$(STACK))
GEN_REVISION := $($GEN_REVISION$(STACK))
BUILD_EXEC := $($BUILD_EXEC$(STACK))
STACK := $(basename $(STACK))

ifneq ($(SUFFIX),)
  app_LIB_SUFFIX := $(app_LIB_SUFFIX)_$(SUFFIX)
endif

##############################################################################
######################### Application Variables ##############################
##############################################################################
#
# source files
TEST_SRC_DIRS    := $(APPLICATION_DIR)/test/src
SRC_DIRS    := $(APPLICATION_DIR)/src
PLUGIN_DIR  := $(APPLICATION_DIR)/plugins $(APPLICATION_DIR)/test/plugins


excluded_srcfiles += main.C
relative_excluded_srcfiles := $(foreach i, $(excluded_srcfiles), $(shell find $(SRC_DIRS) -name $(i)))
ifeq ($(LIBRARY_SUFFIX),yes)
excluded_objects	    := $(patsubst %.C, %_with$(app_LIB_SUFFIX).$(obj-suffix), $(relative_excluded_srcfiles))
else
excluded_objects	    := $(patsubst %.C, %.$(obj-suffix), $(relative_excluded_srcfiles))
endif

find_excludes     := $(foreach i, $(excluded_srcfiles), -not -name $(i))
srcfiles    := $(shell find $(SRC_DIRS) -regex "[^\#~]*\.C" $(find_excludes))


### Unity Build ###
ifeq ($(MOOSE_UNITY),true)

unity_src_dir := $(APPLICATION_DIR)/build/unity_src

# Build unity buiild directory
$(eval $(call unity_dir_rule, $(unity_src_dir)))

# Exclude .libs... but also: exclude unity building src.
# The idea here is that if all they have is src then it's a big jumble of stuff
# that won't benefit from unity building
# Also, exclude the base directory by default because it's another big jumble
# of unrelated stuff
non_unity_dirs := %.libs %/src %src/base $(app_non_unity_dirs)

# Find all of the individual subdirectories
# We will create a Unity file for each individual subdirectory
# The idea is that files grouped withing a subdirectory are closely related
# and will benefit from a Unity build
srcsubdirs := $(shell find $(APPLICATION_DIR)/src -type d -not -path '*/.libs*')

# Filter out the paths we don't want to Unity build
unity_srcsubdirs := $(filter-out $(non_unity_dirs), $(srcsubdirs))
non_unity_srcsubdirs := $(filter $(non_unity_dirs), $(srcsubdirs))

# This is a biggie
# Loop over the subdirectories, creating a rule to create the Unity source file
# for each subdirectory.  To do that we need to create a unique name using the
# full hierarchy of the path underneath src
$(foreach srcsubdir,$(unity_srcsubdirs),$(eval $(call unity_file_rule,$(call unity_unique_name,$(unity_src_dir),$(APPLICATION_DIR),$(srcsubdir)),$(shell find $(srcsubdir) -maxdepth 1 \( -type f -o -type l \) -regex "[^\#~]*\.C"),$(srcsubdir),$(unity_src_dir))))

# This creates the whole list of Unity source files so we can use it as a dependency
app_unity_srcfiles := $(foreach srcsubdir,$(unity_srcsubdirs),$(call unity_unique_name,$(unity_src_dir),$(APPLICATION_DIR),$(srcsubdir)))

# Add to the global list of unity source files
unity_srcfiles += $(app_unity_srcfiles)

# Pick up all of the additional files in the src directory since we're not unity building those
app_non_unity_srcfiles := $(shell find $(non_unity_srcsubdirs) -maxdepth 1 \( -type f -o -type l \) -regex "[^\#~]*\.C" $(find_excludes))

# Override srcfiles
srcfiles    := $(app_unity_srcfiles) $(app_non_unity_srcfiles)
endif



csrcfiles   := $(shell find $(SRC_DIRS) -name "*.c")
fsrcfiles   := $(shell find $(SRC_DIRS) -name "*.f")
f90srcfiles := $(shell find $(SRC_DIRS) -name "*.f90")

# object files
ifeq ($(LIBRARY_SUFFIX),yes)
objects	    := $(patsubst %.C, %_with$(app_LIB_SUFFIX).$(obj-suffix), $(srcfiles))
else
objects	    := $(patsubst %.C, %.$(obj-suffix), $(srcfiles))
endif
cobjects    := $(patsubst %.c, %.$(obj-suffix), $(csrcfiles))
fobjects    := $(patsubst %.f, %.$(obj-suffix), $(fsrcfiles))
f90objects  := $(patsubst %.f90, %.$(obj-suffix), $(f90srcfiles))

app_objects := $(objects) $(cobjects) $(fobjects) $(f90objects) $(ADDITIONAL_APP_OBJECTS)

test_srcfiles    := $(shell find $(TEST_SRC_DIRS) -regex "[^\#~]*\.C" $(find_excludes) 2>/dev/null)
test_csrcfiles   := $(shell find $(TEST_SRC_DIRS) -name "*.c" 2>/dev/null)
test_fsrcfiles   := $(shell find $(TEST_SRC_DIRS) -name "*.f" 2>/dev/null)
test_f90srcfiles := $(shell find $(TEST_SRC_DIRS) -name "*.f90" 2>/dev/null)
ifeq ($(LIBRARY_SUFFIX),yes)
  test_objects:= $(patsubst %.C, %_with$(app_LIB_SUFFIX).$(obj-suffix), $(test_srcfiles))
else
  test_objects:= $(patsubst %.C, %.$(obj-suffix), $(test_srcfiles))
endif

test_cobjects:= $(patsubst %.c, %.$(obj-suffix), $(test_csrcfiles))
test_fobjects:= $(patsubst %.f, %.$(obj-suffix), $(test_fsrcfiles))
test_f90objects:= $(patsubst %.f90, %.$(obj-suffix), $(test_f90srcfiles))
app_test_objects := $(test_objects) $(test_cobjects) $(test_fobjects) $(test_f90objects)

ifeq ($(MOOSE_HEADER_SYMLINKS),true)

$(app_objects): $(moose_config_symlink)
$(app_test_objects): $(moose_config_symlink)
$(excluded_objects): $(moose_config_symlink)

else

$(app_objects): $(moose_config)
$(app_test_objects): $(moose_config)
$(excluded_objects): $(moose_config_symlink)

endif

# plugin files
plugfiles   := $(shell find $(PLUGIN_DIR) -regex "[^\#~]*\.C" 2>/dev/null)
cplugfiles  := $(shell find $(PLUGIN_DIR) -name "*.c" 2>/dev/null)
fplugfiles  := $(shell find $(PLUGIN_DIR) -name "*.f" 2>/dev/null)
f90plugfiles:= $(shell find $(PLUGIN_DIR) -name "*.f90" 2>/dev/null)

# plugins
plugins	    := $(patsubst %.C, %-$(METHOD).plugin, $(plugfiles))
plugins	    += $(patsubst %.c, %-$(METHOD).plugin, $(cplugfiles))
plugins	    += $(patsubst %.f, %-$(METHOD).plugin, $(fplugfiles))
plugins	    += $(patsubst %.f90, %-$(METHOD).plugin, $(f90plugfiles))

# main
MAIN_DIR    ?= $(APPLICATION_DIR)/src
main_src    := $(MAIN_DIR)/main.C
main_object := $(patsubst %.C, %.$(obj-suffix), $(main_src))

# dependency files
app_deps     := $(patsubst %.$(obj-suffix), %.$(obj-suffix).d, $(objects)) \
                $(patsubst %.c, %.$(obj-suffix).d, $(csrcfiles)) \
                $(patsubst %.C, %.$(obj-suffix).d, $(main_src)) \
                $(ADDITIONAL_APP_DEPS)

app_test_deps     := $(patsubst %.$(obj-suffix), %.$(obj-suffix).d, $(test_objects)) \
                $(patsubst %.c, %.$(obj-suffix).d, $(test_csrcfiles))
depend_dirs := $(foreach i, $(DEPEND_MODULES), $(MOOSE_DIR)/modules/$(i)/include)
depend_dirs += $(APPLICATION_DIR)/include
ifneq ($(wildcard $(APPLICATION_DIR)/test/include/*),)
  depend_dirs += $(APPLICATION_DIR)/test/include
endif

# clang static analyzer files
app_analyzer := $(patsubst %.C, %.plist.$(obj-suffix), $(srcfiles))

# library
ifeq ($(LIBRARY_SUFFIX),yes)
  app_LIB     := $(APPLICATION_DIR)/lib/lib$(APPLICATION_NAME)_with$(app_LIB_SUFFIX)-$(METHOD).la
else
  app_LIB     := $(APPLICATION_DIR)/lib/lib$(APPLICATION_NAME)-$(METHOD).la
endif

ifeq ($(LIBRARY_SUFFIX),yes)
  app_test_LIB     := $(APPLICATION_DIR)/test/lib/lib$(APPLICATION_NAME)_with$(app_LIB_SUFFIX)_test-$(METHOD).la
else
  app_test_LIB     := $(APPLICATION_DIR)/test/lib/lib$(APPLICATION_NAME)_test-$(METHOD).la
endif

#
# header symlinks
#
ifeq ($(MOOSE_HEADER_SYMLINKS),true)

include_files	:= $(shell find $(depend_dirs) -regex "[^\#~]*\.[hf]")
all_header_dir := $(APPLICATION_DIR)/build/header_symlinks

# header file links
link_names := $(foreach i, $(include_files), $(all_header_dir)/$(notdir $(i)))

$(eval $(call all_header_dir_rule, $(all_header_dir)))
$(call symlink_rules, $(all_header_dir), $(include_files))

header_symlinks: $(all_header_dir) $(link_names)
app_INCLUDE = -I$(all_header_dir)

else # No Header Symlinks

include_dirs	:= $(shell find $(depend_dirs) -type d)
app_INCLUDE = $(foreach i, $(include_dirs), -I$(i))

endif

# application
app_EXEC    := $(APPLICATION_DIR)/$(APPLICATION_NAME)-$(METHOD)

# revision header
ifeq ($(GEN_REVISION),yes)
  CAMEL_CASE_NAME := $(shell echo $(APPLICATION_NAME) | perl -pe 's/(?:^|_)([a-z])/\u$$1/g')
  app_BASE_DIR    ?= base/
  app_HEADER      ?= $(APPLICATION_DIR)/include/$(app_BASE_DIR)$(CAMEL_CASE_NAME)Revision.h
endif

# depend modules
depend_libs  := $(foreach i, $(DEPEND_MODULES), $(MOOSE_DIR)/modules/$(i)/lib/lib$(i)-$(METHOD).la)

ifeq ($(USE_TEST_LIBS),yes)
  depend_test_libs := $(depend_test_libs) $(app_test_LIB)
  depend_test_libs_flags :=   $(depend_test_libs)
endif


##################################################################################################
# If we are NOT building a module, then make sure the dependency libs are updated to reflect
# all real dependencies
##################################################################################################
ifeq (,$(findstring $(APPLICATION_NAME), $(MODULE_NAMES)))
  depend_libs := $(depend_libs) $(app_LIBS)
endif
# Here we'll filter out MOOSE libs since we'll assume our application already has MOOSE compiled in
depend_libs := $(filter-out $(moose_LIBS),$(depend_libs))
# Create -L/-l versions of the depend libs
depend_libs_flags :=  $(depend_libs)

# If building shared libs, make the plugins a dependency, otherwise don't.
ifeq ($(libmesh_shared),yes)
  app_plugin_deps := $(plugins)
else
  app_plugin_deps :=
endif

app_LIBS       := $(app_LIB) $(app_LIBS)
app_LIBS_other := $(filter-out $(app_LIB),$(app_LIBS))
app_HEADERS    := $(app_HEADER) $(app_HEADERS)
app_INCLUDES   += $(app_INCLUDE) $(ADDITIONAL_INCLUDES)
app_DIRS       += $(APPLICATION_DIR)

# WARNING: the += operator does NOT work here!
ADDITIONAL_CPPFLAGS := $(ADDITIONAL_CPPFLAGS) -D$(shell echo $(APPLICATION_NAME) | perl -pe 'y/a-z/A-Z/' | perl -pe 's/-//g')_ENABLED

# dependencies
-include $(app_deps)
-include $(app_test_deps)

# Rest the certain variables in case this file is sourced again
DEPEND_MODULES :=
SUFFIX :=
LIBRARY_SUFFIX :=


###############################################################################
# Build Rules:
#
###############################################################################

# Instantiate a new suffix rule for the module loader
$(eval $(call CXX_RULE_TEMPLATE,_with$(app_LIB_SUFFIX)))

# If this is a matching module then build the exec, otherwise fall back and use the variable
want_exec := $(BUILD_EXEC)
ifneq (,$(MODULE_NAME))
  ifeq ($(MODULE_NAME),$(APPLICATION_NAME))
    all: $(app_EXEC)
  else
    all: $(app_LIB)
  endif
else
  ifeq ($(BUILD_EXEC),yes)

    # Set a default to install the main application's tests if one isn't set in the Makefile
    ifndef INSTALLABLE_DIRS
      ifneq ($(wildcard $(APPLICATION_DIR)/test/.),)
        INSTALLABLE_DIRS := test/tests->tests
      else
        INSTALLABLE_DIRS := tests
      endif
    endif

    all: $(app_EXEC)
  else
    all: $(app_LIB)
  endif

  BUILD_EXEC :=
endif

app_GIT_DIR := $(shell cd "$(APPLICATION_DIR)" && which git &> /dev/null && git rev-parse --show-toplevel)
app_HEADER_deps := $(realpath $(app_GIT_DIR)/.git/HEAD $(app_GIT_DIR)/.git/index)
ifeq (x$(app_HEADER_deps),x)
  # Files don't exist, this must be a submodule in which case these files are located in master repo's modules directory
  # ".git" in this case is a file, not a folder that contains the real location of the ".git" folder
  app_GIT_DIR := $(realpath $(app_GIT_DIR)/$(shell cut -d' ' -f 2 $(app_GIT_DIR)/.git))
  app_HEADER_deps := $(realpath $(app_GIT_DIR)/HEAD $(app_GIT_DIR)/index)
endif

# Target-specific Variable Values (See GNU-make manual)
$(app_HEADER): curr_dir    := $(APPLICATION_DIR)
$(app_HEADER): curr_app    := $(APPLICATION_NAME)
$(app_HEADER): all_header_dir := $(all_header_dir)
$(app_HEADER): $(app_HEADER_deps)
	@echo "Checking if header needs updating: "$@"..."
	$(shell $(FRAMEWORK_DIR)/scripts/get_repo_revision.py $(curr_dir) $@ $(curr_app))
	@ln -sf $@ $(all_header_dir)

# Target-specific Variable Values (See GNU-make manual)
$(app_LIB): curr_objs := $(app_objects)
$(app_LIB): curr_dir  := $(APPLICATION_DIR)
$(app_LIB): curr_deps := $(depend_libs)
$(app_LIB): curr_libs := $(depend_libs_flags)
$(app_LIB): $(app_HEADER) $(app_plugin_deps) $(depend_libs) $(app_objects) $(ADDITIONAL_DEPEND_LIBS)
	@echo "Linking Library "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(CXXFLAGS) $(libmesh_CXXFLAGS) -o $@ $(curr_objs) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) -rpath $(curr_dir)/lib $(curr_libs)
	@$(libmesh_LIBTOOL) --mode=install --quiet install -c $@ $(curr_dir)/lib

ifeq ($(BUILD_TEST_OBJECTS_LIB),no)
  app_test_LIB :=
  depend_test_libs :=
  depend_test_libs_flags :=
else

# Target-specific Variable Values (See GNU-make manual)
$(app_test_LIB): curr_objs := $(app_test_objects)
$(app_test_LIB): curr_dir  := $(APPLICATION_DIR)/test
$(app_test_LIB): curr_deps := $(depend_libs)
$(app_test_LIB): curr_libs := $(depend_libs_flags)
$(app_test_LIB): $(app_HEADER) $(app_plugin_deps) $(depend_libs) $(app_test_objects) $(ADDITIONAL_DEPEND_LIBS)
	@echo "Linking Library "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(CXXFLAGS) $(libmesh_CXXFLAGS) -o $@ $(curr_objs) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) -rpath $(curr_dir)/lib $(curr_libs)
	@$(libmesh_LIBTOOL) --mode=install --quiet install -c $@ $(curr_dir)/lib
endif

# For static compiles, there are no references to symbols in the module, etc. object archives outside of
# themselves.  This causes linkers to omit the symbols from those archives in the final binary
# which prevents all of the moose objects to not be included/registered in the final binary.  To
# force this we need to ad special linker flags to static builds.  Dynamic builds are immune since
# the linker doesn't know what symbols will be needed by any binary that will dynamically link to
# it in the future - so all symbols remain intact in the dylib/so in that case and are present and
# registered as expected.  See https://stackoverflow.com/questions/5202142/static-variable-initialization-over-a-library
# and https://stackoverflow.com/questions/9459980/c-global-variable-not-initialized-when-linked-through-static-libraries-but-ok#11336506
# for more explanations/details.
uniq = $(if $1,$(firstword $1) $(call uniq,$(filter-out $(firstword $1),$1)))
compilertype := unknown
applibs :=  $(app_test_LIB) $(app_LIBS) $(depend_test_libs)
applibs := $(call uniq,$(applibs))
ifeq ($(libmesh_static),yes)
  ifneq (,$(findstring clang,$(CXX)))
    compilertype := clang
  else
  ifneq (,$(findstring g++,$(CXX)))
    compilertype := gcc
  else
  ifneq (,$(findstring mpicxx,$(CXX)))
    ifneq (,$(findstring clang,$(shell mpicxx -show)))
      compilertype := clang
    else
      compilertype := gcc
    endif
  endif
  endif
  endif

  ifeq ($(compilertype),clang)
    # replace .a with .dylib for testing in dynamic configuration:
    applibs := $(foreach lib,$(patsubst %.la,%.a,$(applibs)),-Wl,-force_load,$(lib))
  else
    applibs := $(foreach lib,$(patsubst %.la,%.a,$(applibs)),-Wl,--whole-archive,$(lib),--no-whole-archive)
  endif
endif

# Codesign command (OS X Only)
codesign :=
ifneq (,$(findstring darwin,$(libmesh_HOST)))
  ifeq (x$(MOOSE_NO_CODESIGN), x)
    get_task_allow_entitlement := $(FRAMEWORK_DIR)/build_support/get_task_allow.plist
    codesign := codesign -s - --entitlements $(get_task_allow_entitlement) $(app_EXEC)
  endif
endif

$(app_EXEC): $(app_LIBS) $(mesh_library) $(main_object) $(app_test_LIB) $(depend_test_libs) $(ADDITIONAL_DEPEND_LIBS)
	@echo "Linking Executable "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(CXXFLAGS) $(libmesh_CXXFLAGS) -o $@ $(main_object) $(depend_test_libs_flags) $(applibs) $(ADDITIONAL_LIBS) $(libmesh_LDFLAGS) $(libmesh_LIBS) $(EXTERNAL_FLAGS)
	@$(codesign)

###### install stuff #############
docs_dir := $(APPLICATION_DIR)/doc
bindst = $(bin_install_dir)/$(notdir $(app_EXEC))
binlink = $(share_install_dir)/$(notdir $(app_EXEC))
# Strip the trailing slashes (if provided) and transform into a suitable Makefile targets
copy_input_targets := $(foreach dir,$(INSTALLABLE_DIRS),target_$(APPLICATION_NAME)_$(patsubst %/,%,$(dir)))

lib_install_targets = $(foreach lib,$(applibs),$(dir $(lib))install_lib_$(notdir $(lib)))
ifneq ($(app_test_LIB),)
	lib_install_targets += $(dir $(app_test_LIB))install_lib_$(notdir $(app_test_LIB))
endif

install_libs:: $(lib_install_targets)

ifneq ($(wildcard $(APPLICATION_DIR)/data/.),)
install_data_$(APPLICATION_NAME)_src := $(APPLICATION_DIR)/data
install_data_$(APPLICATION_NAME)_dst := $(share_install_dir)
install_data:: install_data_$(APPLICATION_NAME)
endif

install_data_%:
	@echo "Installing "$($@_src)"..."
	@mkdir -p $($@_dst)
	@cp -r $($@_src) $($@_dst)

$(copy_input_targets):
	@$(eval kv := $(subst ->, ,$(subst target_$(APPLICATION_NAME)_,,$@)))
	@$(eval source_dir := $(word 1, $(kv)))
	@$(eval dest_dir := $(if $(word 2, $(kv)),$(word 2, $(kv)),$(source_dir)))
	@echo "Installing inputs from directory \"$(source_dir)\" into $(dest_dir)"
	@rm -rf $(share_install_dir)/$(dest_dir)
	@mkdir -p $(share_install_dir)/$(dest_dir)
	@$(eval abs_source_dir := $(realpath $(APPLICATION_DIR)/$(source_dir)))
	@if [ "$(abs_source_dir)" != "" ]; \
	then \
		cp -R $(abs_source_dir)/ $(share_install_dir)/$(dest_dir); \
	else \
		(echo "ERROR: Source directory $(APPLICATION_DIR)/$(source_dir) does not exist!"; exit 1) \
	fi;
	@if [ -e $(APPLICATION_DIR)/testroot ]; \
	then \
		cp -f $(APPLICATION_DIR)/testroot $(share_install_dir)/$(dest_dir)/; \
	elif [ -e $(source_dir)/testroot ]; \
	then \
		cp -f $(source_dir)/testroot $(share_install_dir)/$(dest_dir)/; \
	else \
		echo "app_name = $(APPLICATION_NAME)" > $(share_install_dir)/$(dest_dir)/testroot; \
	fi; \


install_lib_%: % all
	@echo "Installing $<"
	@mkdir -p $(lib_install_dir)
	@$(eval libname := $(shell grep "dlname='.*'" $< | sed -E "s/dlname='(.*)'/\1/g"))
	@$(eval libdst := $(lib_install_dir)/$(libname))
	@cp $(dir $<)$(libname) $(libdst)
	@$(call patch_rpath,$(libdst),../$(lib_install_suffix/.))
	@$(call patch_relink,$(libdst),$(libpath_pcre),$(libname_pcre))
	@$(call patch_relink,$(libdst),$(libpath_framework),$(libname_framework))
	@$(eval libnames := $(foreach lib,$(applibs),$(shell grep "dlname='.*'" $(lib) 2>/dev/null | sed -E "s/dlname='(.*)'/\1/g")))
	@$(eval libpaths := $(foreach lib,$(applibs),$(dir $(lib))$(shell grep "dlname='.*'" $(lib) 2>/dev/null | sed -E "s/dlname='(.*)'/\1/g")))
	@for lib in $(libpaths); do $(call patch_relink,$(libdst),$$lib,$$(basename $$lib)); done

$(binlink): all $(copy_input_targets)
	ln -sf ../../bin/$(notdir $(app_EXEC)) $@

install_$(APPLICATION_NAME)_docs: install_python all
ifeq ($(MOOSE_SKIP_DOCS),)
	@echo "Installing docs"
	@mkdir -p $(docs_install_dir)
	@if [ -f "$(docs_dir)/moosedocs.py" ]; then cd $(docs_dir) && ./moosedocs.py build $(MOOSE_DOCS_FLAGS) --destination $(docs_install_dir); fi
else
	@echo "Skipping docs installation."
endif

$(bindst): $(app_EXEC) all $(copy_input_targets) install_$(APPLICATION_NAME)_docs $(binlink)
	@echo "Installing $<"
	@mkdir -p $(bin_install_dir)
	@cp $< $@
	@$(call patch_rpath,$@,../$(lib_install_suffix)/.)
	@$(eval libnames := $(foreach lib,$(applibs),$(shell grep "dlname='.*'" $(lib) 2>/dev/null | sed -E "s/dlname='(.*)'/\1/g")))
	@$(eval libpaths := $(foreach lib,$(applibs),$(dir $(lib))$(shell grep "dlname='.*'" $(lib) 2>/dev/null | sed -E "s/dlname='(.*)'/\1/g")))
	@for lib in $(libpaths); do $(call patch_relink,$@,$$lib,$$(basename $$lib)); done

ifeq ($(want_exec),yes)
install_bin: $(bindst)
else
install_bin:
endif
####### end install stuff ##############

# Clang static analyzer
sa: $(app_analyzer)

compile_commands_all_srcfiles += $(srcfiles)
