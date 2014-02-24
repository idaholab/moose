#
# MOOSE
#
moose_SRC_DIRS := $(FRAMEWORK_DIR)/src
moose_SRC_DIRS += $(FRAMEWORK_DIR)/contrib/mtwist-1.1
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

# revision header
revision_header = $(FRAMEWORK_DIR)/include/base/HerdRevision.h

app_INCLUDES := $(moose_INCLUDE)
app_LIBS     := $(moose_LIBS)
app_DIRS     := $(FRAMEWORK_DIR)
all:: herd_revision moose

herd_revision:
	$(shell $(FRAMEWORK_DIR)/scripts/get_repo_revision.py $(FRAMEWORK_DIR))

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

-include $(wildcard $(FRAMEWORK_DIR)/contrib/mtwist-1.1/src/*.d)
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

all:: exodiff

exodiff: $(exodiff_APP)

$(exodiff_APP): $(exodiff_objfiles)
	@echo "Linking "$@"..."
	@$(libmesh_LIBTOOL) --tag=CXX $(LIBTOOLFLAGS) --mode=link --quiet \
	  $(libmesh_CXX) $(libmesh_CPPFLAGS) $(libmesh_CXXFLAGS) $(libmesh_INCLUDE) $(exodiff_objfiles) -o $@ $(libmesh_LIBS) $(libmesh_LDFLAGS) $(EXTERNAL_FLAGS)

-include $(wildcard $(exodiff_DIR)/*.d)

#
# Maintenance
#
.PHONY: clean clobber cleanall echo_include echo_library
delete_list := $(moose_LIB) $(exodiff_APP) $(pcre_LIB) libmoose-$(METHOD).* $(pcre_DIR)/libpcre-$(METHOD).*

clean::
	@rm -fr $(delete_list)
	@$(shell find . \( -name "*~" -or -name "*.o" -or -name "*.d" -or -name "*.pyc" -or -name "*.plugin" -or -name "*.mod" \
                           -or -name "*.lo" -or -name "*.la" -or -name "*.plist" \) -exec rm '{}' \;)
	@$(shell find . \( -name *.gch \) | xargs rm -rf)
	@$(shell find . -type d -name .libs | xargs rm -rf) # remove hidden directories created by libtool

clobber::
	@rm -fr $(delete_list)
	@$(shell find . \( -name "*~" -or -name "*.o" -or -name "*.d" -or -name "*.pyc" -or -name "*.plugin" -or -name "*.mod" \
                           -or -name "*.gcda" -or -name "*.gcno" -or -name "*.gcov" -or -name "*.lo" -or -name "*.la" \
			   -or -name "*.plist" \) -exec rm '{}' \;)
	@$(shell find . \( -name *.gch \) | xargs rm -rf)
	@$(shell find . -type d -name .libs | xargs rm -rf) # remove hidden directories created by libtool

cleanall::
	@echo "Cleaning in:\n" $(patsubst %,%"\n",$(app_DIRS))
	@rm -fr $(delete_list)
	@$(shell find $(app_DIRS) \( -name "*~" -or -name "*.o" -or -name "*.d" -or -name "*.pyc" -or -name "*.plugin" -or -name "*.mod" \
                           -or -name "*.lo" -or -name "*.la" -or -name "*.plist" \) -exec rm '{}' \;)
	@$(shell find $(app_DIRS) \( -name *.gch \) | xargs rm -rf)
	@$(shell find $(app_DIRS) -type d -name .libs | xargs rm -rf) # remove hidden directories created by libtool

echo_include:
	@echo $(libmesh_INCLUDE)

echo_library:
	@echo $(app_libraries)
