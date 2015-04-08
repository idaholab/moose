#
# MOOSE
#
moose_SRC_DIRS := $(FRAMEWORK_DIR)/src
moose_SRC_DIRS += $(FRAMEWORK_DIR)/contrib/mtwist
moose_SRC_DIRS += $(FRAMEWORK_DIR)/contrib/dtk_moab

#
# pcre
#
pcre_DIR       := $(FRAMEWORK_DIR)/contrib/pcre
pcre_srcfiles  := $(shell find $(pcre_DIR) -name "*.cc")
pcre_csrcfiles := $(shell find $(pcre_DIR) -name "*.c")
pcre_objects   := $(patsubst %.cc, %.$(obj-suffix), $(pcre_srcfiles))
pcre_objects   += $(patsubst %.c, %.$(obj-suffix), $(pcre_csrcfiles))
pcre_LIB       :=  $(pcre_DIR)/libpcre-$(METHOD).la

moose_INC_DIRS := $(shell find $(FRAMEWORK_DIR)/include -type d -not -path "*/.svn*")
moose_INC_DIRS += $(shell find $(FRAMEWORK_DIR)/contrib/*/include -type d -not -path "*/.svn*")
moose_INCLUDE  := $(foreach i, $(moose_INC_DIRS), -I$(i))

#libmesh_INCLUDE := $(moose_INCLUDE) $(libmesh_INCLUDE)

# Making a .la object instead.  This is what you make out of .lo objects...
moose_LIB := $(FRAMEWORK_DIR)/libmoose-$(METHOD).la

moose_LIBS := $(moose_LIB) $(pcre_LIB)

# source files
moose_precompiled_headers := $(FRAMEWORK_DIR)/include/base/Precompiled.h
moose_srcfiles    := $(shell find $(moose_SRC_DIRS) -name "*.C")
moose_csrcfiles   := $(shell find $(moose_SRC_DIRS) -name "*.c")
moose_fsrcfiles   := $(shell find $(moose_SRC_DIRS) -name "*.f")
moose_f90srcfiles := $(shell find $(moose_SRC_DIRS) -name "*.f90")
# object files
ifdef PRECOMPILED
moose_precompiled_headers_objects := $(patsubst %.h, %.h.gch/$(METHOD).h.gch, $(moose_precompiled_headers))
else
moose_precompiled_headers_objects :=
endif
moose_objects	:= $(patsubst %.C, %.$(obj-suffix), $(moose_srcfiles))
moose_objects	+= $(patsubst %.c, %.$(obj-suffix), $(moose_csrcfiles))
moose_objects   += $(patsubst %.f, %.$(obj-suffix), $(moose_fsrcfiles))
moose_objects   += $(patsubst %.f90, %.$(obj-suffix), $(moose_f90srcfiles))
# dependency files
moose_deps := $(patsubst %.C, %.$(obj-suffix).d, $(moose_srcfiles)) \
              $(patsubst %.c, %.$(obj-suffix).d, $(moose_csrcfiles))

# clang static analyzer files
moose_analyzer := $(patsubst %.C, %.plist.$(obj-suffix), $(moose_srcfiles))

app_INCLUDES := $(moose_INCLUDE)
app_LIBS     := $(moose_LIBS)
app_DIRS     := $(FRAMEWORK_DIR)
all:: moose_revision moose

# revision header
moose_revision_header = $(FRAMEWORK_DIR)/include/base/MooseRevision.h
moose_revision:
  $(shell $(FRAMEWORK_DIR)/scripts/get_repo_revision.py $(FRAMEWORK_DIR) \
    $(moose_revision_header) MOOSE)

moose: $(moose_LIB)

# [JWP] With libtool, there is only one link command, it should work whether you are creating
# shared or static libraries, and it should be portable across Linux and Mac...
$(pcre_LIB): $(pcre_objects)
	@echo "Linking Library "$@"..."
	@$(libmesh_LIBTOOL) --tag=CC $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CC) $(libmesh_CFLAGS) -o $@ $(pcre_objects) $(libmesh_LIBS) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) -rpath $(pcre_DIR)
	@$(libmesh_LIBTOOL) --mode=install --quiet install -c $(pcre_LIB) $(pcre_DIR)


$(moose_LIB): $(moose_precompiled_headers_objects) $(moose_objects) $(pcre_LIB)
	@echo "Linking Library "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(libmesh_CXXFLAGS) -o $@ $(moose_objects) $(pcre_LIB) $(libmesh_LIBS) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS) -rpath $(FRAMEWORK_DIR)
	@$(libmesh_LIBTOOL) --mode=install --quiet install -c $(moose_LIB) $(FRAMEWORK_DIR)

## Clang static analyzer
sa:: $(moose_analyzer)

# include MOOSE dep files. Note: must use -include for deps, since they don't exist for first time builds.
-include $(moose_deps)

-include $(wildcard $(FRAMEWORK_DIR)/contrib/mtwist/src/*.d)
-include $(wildcard $(FRAMEWORK_DIR)/contrib/dtk_moab/src/*.d)
-include $(wildcard $(FRAMEWORK_DIR)/contrib/pcre/src/*.d)

ifdef PRECOMPILED
-include $(FRAMEWORK_DIR)/include/base/Precompiled.h.gch/$(METHOD).h.gch.d
endif

#
# exodiff
#
exodiff_DIR := $(FRAMEWORK_DIR)/contrib/exodiff
exodiff_APP := $(exodiff_DIR)/exodiff
exodiff_srcfiles := $(shell find $(exodiff_DIR) -name "*.C")
exodiff_objfiles := $(patsubst %.C, %.$(obj-suffix), $(exodiff_srcfiles))
exodiff_includes := $(app_INCLUDES) -I$(exodiff_DIR) $(libmesh_INCLUDE)

all:: exodiff

# Target-specific Variable Values (See GNU-make manual)
exodiff: app_INCLUDES := $(exodiff_includes)
exodiff: $(exodiff_APP)

$(exodiff_APP): $(exodiff_objfiles)
	@echo "Linking "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(libmesh_CPPFLAGS) $(libmesh_CXXFLAGS) $(libmesh_INCLUDE) $(exodiff_objfiles) -o $@ $(libmesh_LIBS) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS)

-include $(wildcard $(exodiff_DIR)/*.d)

#
# Clean targets
#
.PHONY: clean clobber cleanall echo_include echo_library

# Set up app-specific variables for MOOSE, so that it can use the same clean target as the apps
app_LIB := $(moose_LIBS) $(exodiff_APP) libmoose-$(METHOD).*
app_objects := $(moose_objects)
app_deps := $(moose_deps)

# The clean target removes everything we can remove "easily",
# i.e. stuff which we have Makefile variables for.  Notes:
# .) This clean target is also used by the apps, so it should only refer to
#    app-specific variables.
# .) This target respects $(METHOD), so, for example 'METHOD=dbg make
#    clean' will only clean debug object and executable files.
# .) Calling 'make clean' in an app should not remove MOOSE object
#    files, libraries, etc.
clean::
	@rm -rf $(app_LIB) $(app_EXEC) $(app_objects) $(main_object) $(app_deps) $(app_HEADER)

# The clobber target does 'make clean' and then uses 'find' to clean a
# bunch more stuff.  We have to write this target as though it could
# be called from a top level application, therefore, we prune the
# following paths from the search:
# .) moose (ignore a possible MOOSE submodule)
# .) .git  (don't accidentally delete any of git's metadata)
# .) .svn  (don't accidentally delete any of svn's metadata)
# Notes:
# .) -exec rm is the only way to delete stuff, it won't work right if
#    you pipe the output of find to 'xargs rm -rf' or pass the -delete
#    flag to find
# .) The ./ in front of the path names is absolutely required for the
#    find command to work correctly.
# .) Be careful: running 'make -n clobber' will actually delete files!
# .) Running 'make clobber' is a good way to clean up outdated
#    dependency and object files when you upgrade OSX versions or as
#    source files are deleted over time.
# .) 'make clobber' does not respect $(METHOD), it just deletes
#    everything it can find!
# .) We send any errors from the find command to /dev/null, since we
#    don't really care about them and find has this annoying "feature"
#    where it tries to search in directories it has already deleted (in
#    this case .libs) and prints an error message about it.
clobber:: clean
	$(shell find . \( -path ./moose -or -path ./.git -or -path ./.svn \) -prune -or \
          \( -name "*~" -or -name "*.lo" -or -name "*.la" -or -name "*.dylib" -or -name "*.so*" -or -name "*.a" \
          -or -name "*-opt" -or -name "*-dbg" -or -name "*-oprof" \
          -or -name "*.d" -or -name "*.pyc" -or -name "*.plugin" -or -name "*.mod" -or -name "*.plist" \
          -or -name "*.gcda" -or -name "*.gcno" -or -name "*.gcov" -or -name "*.gch" -or -name .libs \) \
          -exec rm -rf '{}' \; 2>/dev/null)

# cleanall runs 'make clean' in all dependent application directories
cleanall:: clean
	@echo "Cleaning in:"
	@for dir in $(app_DIRS); do \
          echo \ $$dir; \
          make -C $$dir clean ; \
        done

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
