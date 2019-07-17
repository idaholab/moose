# Whether or not to do a Unity build
MOOSE_UNITY ?= true
MOOSE_HEADER_SYMLINKS ?= true

#
# MOOSE
#
APPLICATION_DIR := $(FRAMEWORK_DIR)
moose_SRC_DIRS := $(FRAMEWORK_DIR)/src
moose_SRC_DIRS += $(FRAMEWORK_DIR)/contrib/mtwist
moose_SRC_DIRS += $(FRAMEWORK_DIR)/contrib/jsoncpp

#
# pcre
#
pcre_DIR       := $(FRAMEWORK_DIR)/contrib/pcre
pcre_srcfiles  := $(shell find $(pcre_DIR) -name "*.cc")
pcre_csrcfiles := $(shell find $(pcre_DIR) -name "*.c")
pcre_objects   := $(patsubst %.cc, %.$(obj-suffix), $(pcre_srcfiles))
pcre_objects   += $(patsubst %.c, %.$(obj-suffix), $(pcre_csrcfiles))
pcre_LIB       :=  $(pcre_DIR)/libpcre-$(METHOD).la
# dependency files
pcre_deps      := $(patsubst %.cc, %.$(obj-suffix).d, $(pcre_srcfiles)) \

#
# hit (new getpot parser)
#
hit_DIR       := $(FRAMEWORK_DIR)/contrib/hit
hit_srcfiles  := $(hit_DIR)/parse.cc $(hit_DIR)/lex.cc $(hit_DIR)/braceexpr.cc
hit_objects   := $(patsubst %.cc, %.$(obj-suffix), $(hit_srcfiles))
hit_LIB       := $(hit_DIR)/libhit-$(METHOD).la
# dependency files
hit_deps      := $(patsubst %.cc, %.$(obj-suffix).d, $(hit_srcfiles))

#
# hit python bindings
#
pyhit_srcfiles  := $(hit_DIR)/hit.cpp $(hit_DIR)/lex.cc $(hit_DIR)/parse.cc $(hit_DIR)/braceexpr.cc
pyhit_LIB       := $(FRAMEWORK_DIR)/../python/hit.so

# some systems have python2 but no python2-config command - fall back to python-config for them
pyconfig := python3-config
ifeq (, $(shell which python3-config 2>/dev/null))
  pyconfig := python-config
endif

hit $(pyhit_LIB): $(pyhit_srcfiles)
	@echo "Building and linking "$@"..."
	@bash -c '(cd "$(hit_DIR)" && $(libmesh_CXX) -std=c++11 -w -fPIC -lstdc++ -shared -L`$(pyconfig) --prefix`/lib `$(pyconfig) --includes` `$(pyconfig) --ldflags` $^ -o $(pyhit_LIB))'

#
# gtest
#
gtest_DIR       := $(FRAMEWORK_DIR)/contrib/gtest
gtest_srcfiles  := $(gtest_DIR)/gtest-all.cc
gtest_objects   := $(patsubst %.cc, %.$(obj-suffix), $(gtest_srcfiles))
gtest_LIB       := $(gtest_DIR)/libgtest.la
# dependency files
gtest_deps      := $(patsubst %.cc, %.$(obj-suffix).d, $(gtest_srcfiles))

#
# MooseConfigure
#
moose_config := $(FRAMEWORK_DIR)/include/base/MooseConfig.h
moose_default_config := $(FRAMEWORK_DIR)/include/base/MooseDefaultConfig.h

$(moose_config):
	@echo "Copying default MOOSE configuration to: "$@"..."
	@cp $(moose_default_config) $(moose_config)

#
# header symlinks
#
ifeq ($(MOOSE_HEADER_SYMLINKS),true)

all_header_dir := $(FRAMEWORK_DIR)/build/header_symlinks
moose_all_header_dir := $(all_header_dir)

define all_header_dir_rule
$(1):
	@echo Rebuilding symlinks in $$@
	@$$(shell mkdir -p $$@)
endef

include_files	:= $(shell find $(FRAMEWORK_DIR)/include \( -regex "[^\#~]*\.h" ! -name "*MooseConfig.h" \))
link_names := $(foreach i, $(include_files), $(all_header_dir)/$(notdir $(i)))

# Create a rule for one symlink for one header file
# Args
# 1: the header file
# 2: the symlink to create
define symlink_rule
$(2): $(1)
	@ln -sf $$< $$@
endef

# Create a rule for a symlink for each header file
# Args:
# 1: all_header_dir
# 2: list of header files
define symlink_rules
$(foreach i, $(2), $(eval $(call symlink_rule, $(i), $(1)/$(notdir $(i)))))
endef

$(eval $(call all_header_dir_rule, $(all_header_dir)))
$(call symlink_rules, $(all_header_dir), $(include_files))

moose_config_symlink := $(moose_all_header_dir)/MooseConfig.h
$(moose_config_symlink): $(moose_config) | $(moose_all_header_dir)
	@echo "Symlinking MOOSE configure "$(moose_config_symlink)
	@ln -sf $(moose_config) $(moose_config_symlink)

header_symlinks: $(all_header_dir) $(link_names)
moose_INC_DIRS := $(all_header_dir)

else # No Header Symlinks

moose_INC_DIRS := $(shell find $(FRAMEWORK_DIR)/include -type d)

endif

moose_INC_DIRS += $(shell find $(FRAMEWORK_DIR)/contrib/*/include -type d)
moose_INC_DIRS += "$(gtest_DIR)"
moose_INC_DIRS += "$(hit_DIR)"
moose_INCLUDE  := $(foreach i, $(moose_INC_DIRS), -I$(i))

#libmesh_INCLUDE := $(moose_INCLUDE) $(libmesh_INCLUDE)

# Making a .la object instead.  This is what you make out of .lo objects...
moose_LIB := $(FRAMEWORK_DIR)/libmoose-$(METHOD).la

moose_LIBS := $(moose_LIB) $(pcre_LIB) $(hit_LIB)

### Unity Build ###
ifeq ($(MOOSE_UNITY),true)

srcsubdirs := $(shell find $(FRAMEWORK_DIR)/src -type d -not -path '*/.libs*')

moose_non_unity := %/base %/utils

unity_src_dir := $(FRAMEWORK_DIR)/build/unity_src

unity_srcsubdirs := $(filter-out $(moose_non_unity), $(srcsubdirs))
non_unity_srcsubdirs := $(filter $(moose_non_unity), $(srcsubdirs))

define unity_dir_rule
$(1):
	@echo Creating Unity Directory $$@
	@mkdir -p $(1)
endef

$(eval $(call unity_dir_rule, $(unity_src_dir)))

# 1: the unity file to build
# 2: the source files in that unity file (We sort these for consistent builds across platforms)
# 3: The unity source directory
# 4: The unity build area (for filtering out, not a dependency)
# The "|" in the prereqs starts the beginning of "position dependent" prereqs
# these are prereqs that must be run first - but their timestamp isn't used
define unity_file_rule
$(1):$(2) $(3) | $(4)
	@echo Creating Unity $$@
	$$(shell echo > $$@)
	$$(foreach srcfile,$$(sort $$(filter-out $(3) $(4),$$^)),$$(shell echo '#include"$$(srcfile)"' >> $$@))
endef

# 1: The directory where the unity source files will go
# 2: The application directory
# 3: A subdir that will be unity built
# The output: is the unique .C file name for the unity file
# Here's what this does:
# For each explicit path we're going to create a unique unity filename by:
# 1. Stripping off $(FRAMEWORK_DIR)/src
# 2. For the special case of src itself strip off $(FRAMEWORK_DIR)
# 3. Turn every remaining '/' into an '_'
# 4. Add '_Unity.C'
unity_unique_name = $(1)/$(subst /,_,$(patsubst $(2)/%,%,$(patsubst $(2)/src/%,%,$(3))))_Unity.C

# Here's what this does:
# 1. Defines a rule to build a unity file from each subdirectory under src
# 2. Does that by looping over the explicit paths found before (unity_srcsubdirs)
# 3. For each explicit path we're going to create a unique unity filename by calling unity_unique_name
# 4. Now that we have the name of the Unity file we need to find all of the .C files that should be #included in it
# 4a. Use find to pick up all .C files
# 4b. Make sure we don't pick up any _Unity.C files (we shouldn't have any anyway)
$(foreach srcsubdir,$(unity_srcsubdirs),$(eval $(call unity_file_rule,$(call unity_unique_name,$(unity_src_dir),$(FRAMEWORK_DIR),$(srcsubdir)),$(shell find $(srcsubdir) -maxdepth 1 \( -type f -o -type l \) -name "*.C"),$(srcsubdir),$(unity_src_dir))))

app_unity_srcfiles := $(foreach srcsubdir,$(unity_srcsubdirs),$(call unity_unique_name,$(unity_src_dir),$(FRAMEWORK_DIR),$(srcsubdir)))

#$(info $(app_unity_srcfiles))

unity_srcfiles += $(app_unity_srcfiles)

moose_srcfiles    := $(app_unity_srcfiles) $(shell find $(non_unity_srcsubdirs) -maxdepth 1 -regex "[^\#~]*\.C") $(shell find $(filter-out %/src,$(moose_SRC_DIRS)) -regex "[^\#~]*\.C")

else # Non-Unity
moose_srcfiles    := $(shell find $(moose_SRC_DIRS) -regex "[^\#~]*\.C")
endif

# source files
moose_csrcfiles   := $(shell find $(moose_SRC_DIRS) -name "*.c")
moose_fsrcfiles   := $(shell find $(moose_SRC_DIRS) -name "*.f")
moose_f90srcfiles := $(shell find $(moose_SRC_DIRS) -name "*.f90")
# object files
moose_objects	:= $(patsubst %.C, %.$(obj-suffix), $(moose_srcfiles))
moose_objects	+= $(patsubst %.c, %.$(obj-suffix), $(moose_csrcfiles))
moose_objects   += $(patsubst %.f, %.$(obj-suffix), $(moose_fsrcfiles))
moose_objects   += $(patsubst %.f90, %.$(obj-suffix), $(moose_f90srcfiles))
# dependency files
moose_deps := $(patsubst %.C, %.$(obj-suffix).d, $(moose_srcfiles)) \
              $(patsubst %.c, %.$(obj-suffix).d, $(moose_csrcfiles))

# clang static analyzer files
moose_analyzer := $(patsubst %.C, %.plist.$(obj-suffix), $(moose_srcfiles))
moose_analyzer += $(patsubst %.cc, %.plist.$(obj-suffix), $(hit_srcfiles))

app_INCLUDES := $(moose_INCLUDE) $(libmesh_INCLUDE)
app_LIBS     := $(moose_LIBS)
app_DIRS     := $(FRAMEWORK_DIR)

moose_revision_header := $(FRAMEWORK_DIR)/include/base/MooseRevision.h

all: libmesh_submodule_status header_symlinks $(moose_revision_header) moose

# revision header
moose_GIT_DIR := $(shell cd "$(FRAMEWORK_DIR)" && which git &> /dev/null && git rev-parse --show-toplevel)
# Use wildcard in case the files don't exist
moose_HEADER_deps := $(wildcard $(moose_GIT_DIR)/.git/HEAD $(moose_GIT_DIR)/.git/index)

$(moose_revision_header): $(moose_HEADER_deps)
	@echo "Checking if header needs updating: "$@"..."
	$(shell $(FRAMEWORK_DIR)/scripts/get_repo_revision.py $(FRAMEWORK_DIR) \
	  $(moose_revision_header) MOOSE)
	@if [ ! -e "$(moose_all_header_dir)/MooseRevision.h" ]; then \
		ln -sf $(moose_revision_header) $(moose_all_header_dir); \
	fi

# libmesh submodule status
libmesh_status := $(shell git -C $(MOOSE_DIR) submodule status 2>/dev/null | grep libmesh | cut -c1)
ifneq (,$(findstring +,$(libmesh_status)))
  ifneq ($(origin LIBMESH_DIR),environment)
    libmesh_message = "\n***WARNING***\nYour libmesh is out of date.\nYou need to run update_and_rebuild_libmesh.sh in the scripts directory.\n\n"
  endif
endif
libmesh_submodule_status:
	@if [ x$(libmesh_message) != "x" ]; then printf $(libmesh_message); fi

moose: $(moose_LIB)

# [JWP] With libtool, there is only one link command, it should work whether you are creating
# shared or static libraries, and it should be portable across Linux and Mac...
$(pcre_LIB): $(pcre_objects)
	@echo "Linking Library "$@"..."
	@$(libmesh_LIBTOOL) --tag=CC $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CC) $(libmesh_CFLAGS) -o $@ $(pcre_objects) $(libmesh_LIBS) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) -rpath $(pcre_DIR)
	@$(libmesh_LIBTOOL) --mode=install --quiet install -c $(pcre_LIB) $(pcre_DIR)

$(gtest_LIB): $(gtest_objects)
	@echo "Linking Library "$@"..."
	@$(libmesh_LIBTOOL) --tag=CC $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CC) $(libmesh_CFLAGS) -o $@ $(gtest_objects) $(libmesh_LIBS) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) -rpath $(gtest_DIR)
	@$(libmesh_LIBTOOL) --mode=install --quiet install -c $(gtest_LIB) $(gtest_DIR)

$(hit_LIB): $(hit_objects)
	@echo "Linking Library "$@"..."
	@$(libmesh_LIBTOOL) --tag=CC $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(CXXFLAGS) $(libmesh_CXXFLAGS) -o $@ $(hit_objects) $(libmesh_LIBS) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) -rpath $(hit_DIR)
	@$(libmesh_LIBTOOL) --mode=install --quiet install -c $(hit_LIB) $(hit_DIR)

$(moose_LIB): $(moose_objects) $(pcre_LIB) $(gtest_LIB) $(hit_LIB) $(pyhit_LIB)
	@echo "Linking Library "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(CXXFLAGS) $(libmesh_CXXFLAGS) -o $@ $(moose_objects) $(pcre_LIB) $(png_LIB) $(libmesh_LIBS) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) -rpath $(FRAMEWORK_DIR)
	@$(libmesh_LIBTOOL) --mode=install --quiet install -c $(moose_LIB) $(FRAMEWORK_DIR)

ifeq ($(MOOSE_HEADER_SYMLINKS),true)

$(moose_objects): $(moose_config_symlink)

else

$(moose_objects): $(moose_config)

endif

## Clang static analyzer
sa: $(moose_analyzer)

# include MOOSE dep files. Note: must use -include for deps, since they don't exist for first time builds.
-include $(moose_deps)

-include $(wildcard $(FRAMEWORK_DIR)/contrib/mtwist/src/*.d)
-include $(wildcard $(FRAMEWORK_DIR)/contrib/jsoncpp/src/*.d)
-include $(wildcard $(FRAMEWORK_DIR)/contrib/pcre/src/*.d)
-include $(wildcard $(FRAMEWORK_DIR)/contrib/gtest/*.d)
-include $(wildcard $(FRAMEWORK_DIR)/contrib/hit/*.d)

#
# exodiff
#
exodiff_DIR := $(FRAMEWORK_DIR)/contrib/exodiff
exodiff_APP := $(exodiff_DIR)/exodiff
exodiff_srcfiles := $(shell find $(exodiff_DIR) -name "*.C")
exodiff_objects  := $(patsubst %.C, %.$(obj-suffix), $(exodiff_srcfiles))
exodiff_includes := -I$(exodiff_DIR) $(libmesh_INCLUDE)
# dependency files
exodiff_deps := $(patsubst %.C, %.$(obj-suffix).d, $(exodiff_srcfiles))

all: exodiff

# Target-specific Variable Values (See GNU-make manual)
exodiff: app_INCLUDES := $(exodiff_includes)
exodiff: libmesh_CXXFLAGS := -std=gnu++11 -O2 -felide-constructors -w
exodiff: $(exodiff_APP)
$(exodiff_APP): $(exodiff_objects)
	@echo "Linking Executable "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(libmesh_CPPFLAGS) $(CXXFLAGS) $(libmesh_CXXFLAGS) $(libmesh_INCLUDE) $(exodiff_objects) -o $@ $(libmesh_LIBS) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS)

-include $(wildcard $(exodiff_DIR)/*.d)

#
# Clean targets
#
.PHONY: clean clobber cleanall echo_include echo_library libmesh_submodule_status hit

# Set up app-specific variables for MOOSE, so that it can use the same clean target as the apps
app_EXEC := $(exodiff_APP)
app_LIB  := $(moose_LIBS) $(pcre_LIB) $(gtest_LIB) $(hit_LIB) $(pyhit_LIB)
app_objects := $(moose_objects) $(exodiff_objects) $(pcre_objects) $(gtest_objects) $(hit_objects)
app_deps := $(moose_deps) $(exodiff_deps) $(pcre_deps) $(gtest_deps) $(hit_deps)

# The clean target removes everything we can remove "easily",
# i.e. stuff which we have Makefile variables for.  Notes:
# .) This clean target is also used by the apps, so it should only refer to
#    app-specific variables.
# .) This target respects $(METHOD), so, for example 'METHOD=dbg make
#    clean' will only clean debug object and executable files.
# .) Calling 'make clean' in an app should not remove MOOSE object
#    files, libraries, etc.
clean:
	@$(libmesh_LIBTOOL) --mode=uninstall --quiet rm -f $(app_LIB) $(app_test_LIB)
	@rm -rf $(app_EXEC) $(app_objects) $(main_object) $(app_deps) $(app_HEADER) $(app_test_objects) $(app_unity_srcfiles)
	@rm -rf $(APPLICATION_DIR)/build

# The clobber target does 'make clean' and then uses 'find' to clean a
# bunch more stuff.  We have to write this target as though it could
# be called from a top level application, therefore, we prune the
# following paths from the search:
# .) moose (ignore a possible MOOSE submodule)
# .) .git  (don't accidentally delete any of git's metadata)
# Notes:
# .) Be careful: running 'make -n clobber' will actually delete files!
# .) 'make clobber' does not respect $(METHOD), it just deletes
#    everything it can find!
# .) Running 'make clobberall' is a good way to clean up outdated
#    dependency and object files when you upgrade OSX versions or as
#    source files are deleted over time.
clobber: clean
	@$(MOOSE_DIR)/scripts/clobber.py -v $(CURDIR)
	@rm -rf $(APPLICATION_DIR)/build

# cleanall runs 'make clean' in all dependent application directories
cleanall: clean
	@echo "Cleaning in:"
	@for dir in $(app_DIRS); do \
          echo \ $$dir; \
          make -C $$dir clean ; \
        done

# clobberall runs 'make clobber' in all dependent application directories
clobberall: clobber
	@echo "Clobbering in:"
	@for dir in $(app_DIRS); do \
          echo \ $$dir; \
          make -C $$dir clobber ; \
        done

# clang_complete builds a clang configuration file for various clang-based autocompletion plugins
.clang_complete:
	@echo "Building .clang_complete file"
	@echo "-xc++" > .clang_complete
	@echo "-std=c++11" >> .clang_complete
	@for item in $(libmesh_CPPFLAGS) $(CXXFLAGS) $(libmesh_CXXFLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) $(ADDITIONAL_INCLUDES); do \
          echo $$item >> .clang_complete;  \
        done

compile_commands_all_srcfiles := $(moose_srcfiles) $(srcfiles)
compile_commands.json:
ifeq (4.0,$(firstword $(sort $(MAKE_VERSION) 4.0)))
	$(file > .compile_commands.json,$(CURDIR))
	$(file >> .compile_commands.json,$(libmesh_CXX))
	$(file >> .compile_commands.json,$(libmesh_CPPFLAGS) $(CXXFLAGS) $(libmesh_CXXFLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) $(ADDITIONAL_INCLUDES))
	$(file >> .compile_commands.json,$(compile_commands_all_srcfiles))
else
	@echo $(CURDIR) > .compile_commands.json
	@echo $(libmesh_CXX) >> .compile_commands.json
	@echo $(libmesh_CPPFLAGS) $(CXXFLAGS) $(libmesh_CXXFLAGS) $(app_INCLUDES) $(libmesh_INCLUDE) $(ADDITIONAL_INCLUDES) >> .compile_commands.json
	@echo $(compile_commands_all_srcfiles) >> .compile_commands.json
endif
	@ $(FRAMEWORK_DIR)/scripts/compile_commands.py < .compile_commands.json > compile_commands.json
	@rm .compile_commands.json

# Debugging stuff
echo_include:
	@echo $(app_INCLUDES) $(libmesh_INCLUDE)

echo_app_libs:
	@echo $(app_LIBS)

echo_app_lib:
	@echo $(app_LIB)

echo_app_exec:
	@echo $(app_EXEC)

echo_app_objects:
	@echo $(app_objects)

echo_app_deps:
	@echo $(app_deps)
