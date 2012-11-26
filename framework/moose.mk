#
# MOOSE
#
moose_SRC_DIRS := $(MOOSE_DIR)/src
moose_SRC_DIRS += $(MOOSE_DIR)/contrib/mtwist-1.1

moose_INC_DIRS := $(shell find $(MOOSE_DIR)/include -type d -not -path "*/.svn*")
moose_INC_DIRS += $(shell find $(MOOSE_DIR)/contrib/*/include -type d -not -path "*/.svn*")
moose_INCLUDE  := $(foreach i, $(moose_INC_DIRS), -I$(i))

libmesh_INCLUDE := $(moose_INCLUDE) $(libmesh_INCLUDE)

moose_LIB := $(MOOSE_DIR)/libmoose-$(METHOD)$(libext)
LIBS += $(moose_LIB)

# source filese
moose_precompiled_headers := $(MOOSE_DIR)/include/base/Precompiled.h
#$(shell find $(MOOSE_DIR)/include/base -name *.h)
moose_srcfiles    := $(shell find $(moose_SRC_DIRS) -name *.C)
moose_csrcfiles   := $(shell find $(moose_SRC_DIRS) -name *.c)
moose_fsrcfiles   := $(shell find $(moose_SRC_DIRS) -name *.f)
moose_f90srcfiles := $(shell find $(moose_SRC_DIRS) -name *.f90)
# object files
ifdef PRECOMPILED
moose_precompiled_headers_objects := $(patsubst %.h, %.h.gch/$(METHOD).h.gch, $(moose_precompiled_headers))
else
moose_precompiled_headers_objects := 
endif
moose_objects	:= $(patsubst %.C, %.$(obj-suffix), $(moose_srcfiles))
moose_objects	+= $(patsubst %.c, %.$(obj-suffix), $(moose_csrcfiles))
moose_objects += $(patsubst %.f, %.$(obj-suffix), $(moose_fsrcfiles))
moose_objects += $(patsubst %.f90, %.$(obj-suffix), $(moose_f90srcfiles))
# dependency files
moose_deps := $(patsubst %.C, %.$(obj-suffix).d, $(moose_srcfiles)) \
              $(patsubst %.c, %.$(obj-suffix).d, $(moose_csrcfiles))

# revision header
revision_header = $(MOOSE_DIR)/include/base/HerdRevision.h

all:: herd_revision moose

herd_revision:
	$(shell $(MOOSE_DIR)/scripts/get_repo_revision.py $(MOOSE_DIR))

moose: $(moose_LIB) 

# build rule for MOOSE
ifeq ($(enable-shared),yes)
# Build dynamic library
$(moose_LIB): $(moose_precompiled_headers_objects) $(moose_objects)
	@echo "Linking "$@"..."
	@$(libmesh_CC) $(libmesh_CXXSHAREDFLAG) -o $@ $(moose_objects) $(libmesh_LDFLAGS)
else
# Build static library
ifeq ($(findstring darwin,$(hostos)),darwin)
$(moose_LIB): $(moose_precompiled_headers_objects) $(moose_objects)
	@echo "Linking "$@"..."
	@libtool -static -o $@ $(moose_objects)
else
$(moose_LIB): $(moose_precompiled_headers_objects) $(moose_objects)
	@echo "Linking "$@"..."
	@$(AR) rv $@ $(moose_objects)
endif
endif

# include MOOSE dep files
-include $(moose_deps)
-include $(MOOSE_DIR)/contrib/mtwist-1.1/src/*.d
ifdef PRECOMPILED
-include $(MOOSE_DIR)/include/base/Precompiled.h.gch/$(METHOD).h.gch.d
endif

#
# exodiff
#
exodiff_DIR := $(MOOSE_DIR)/contrib/exodiff
exodiff_APP := $(exodiff_DIR)/exodiff
exodiff_srcfiles := $(shell find $(exodiff_DIR) -name *.C)
exodiff_objfiles := $(patsubst %.C, %.$(obj-suffix), $(exodiff_srcfiles))

all:: exodiff

exodiff: $(exodiff_APP)

$(exodiff_APP): $(exodiff_objfiles)
	@echo "Linking "$@"..."
	@$(libmesh_CXX) $(libmesh_CXXFLAGS) $(exodiff_objfiles) -o $@ $(libmesh_LIBS) $(libmesh_LDFLAGS)

-include $(exodiff_DIR)/*.d


#
# Maintenance
#
delete_list := $(moose_LIB) $(exodiff_APP)

clean::
	@rm -fr $(delete_list)
	@find . \( -name "*~" -or -name "*.o" -or -name "*.d" -or -name "*.pyc" -or -name "*.plugin" -or -name "*.mod" \) -exec rm '{}' \;
	@find . \( -name *.gch \) | xargs rm -rf

clobber::
	@rm -fr $(delete_list)
	@find . \( -name "*~" -or -name "*.o" -or -name "*.d" -or -name "*.pyc" -or -name "*.plugin" -or -name "*.mod" \
                -or -name "*.gcda" -or -name "*.gcno" -or -name "*.gcov" \) -exec rm '{}' \;
	@find . \( -name *.gch \) | xargs rm -rf

cleanall::
	make -C $(MOOSE_DIR) clean


# echo:
# 	@echo $(MOOSE_DIR)/include/base/Precompiled.h.gch.d
