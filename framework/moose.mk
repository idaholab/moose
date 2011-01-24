moose_DIRS	:= $(shell find $(MOOSE_DIR)/include -type d -not -path "*/.svn*")
contrib_DIRS  := $(shell find $(MOOSE_DIR)/contrib/*/include -type d -not -path "*/.svn*")
moose_INCLUDE 	:= $(foreach i, $(moose_DIRS) $(contrib_DIRS), -I$(i))

moose_LIB := $(MOOSE_DIR)/libmoose-$(METHOD)$(static_libext)
ifeq ($(enable-shared),yes)
	moose_LIB := $(MOOSE_DIR)/libmoose-$(METHOD)$(shared_libext)
endif

libmesh_INCLUDE += $(moose_INCLUDE)
LIBS += $(moose_LIB)
ifeq ($(enable-shared),yes)
	LIBS += -Wl,-rpath,$(MOOSE_DIR)
endif

# source files
moose_srcfiles    := $(shell find $(MOOSE_DIR) -name *.C)
moose_fsrcfiles   := $(shell find $(MOOSE_DIR) -name *.f)
moose_f90srcfiles := $(shell find $(MOOSE_DIR) -name *.f90)

# object files
moose_objects	    := $(patsubst %.C, %.$(obj-suffix), $(moose_srcfiles))
moose_fobjects    := $(patsubst %.f, %.$(obj-suffix), $(moose_fsrcfiles))
moose_f90objects  := $(patsubst %.f90, %.$(obj-suffix), $(moose_f90srcfiles))

$(moose_LIB): $(moose_objects) $(moose_fobjects) $(moose_f90objects)
	$(MAKE) -C $(MOOSE_DIR)

-include $(MOOSE_DIR)/src/*/*.d
-include $(MOOSE_DIR)/src/*/*/*.d

$(target): $(moose_LIB)

cleanall:: moose_clean

moose_clean:
	$(MAKE) -C $(MOOSE_DIR) clean

