CROW_DIR := $(ROOT_DIR)/crow

PYTHON3_HELLO := $(shell python3 -c "print('HELLO')" 2>/dev/null)
PYTHON2_HELLO := $(shell python -c "print 'HELLO'" 2>/dev/null)

SWIG_VERSION := $(shell swig -version 2>/dev/null)
PYTHON_CONFIG_WHICH := $(shell which python-config 2>/dev/null)

UNAME := $(shell uname)

ifneq ($(PYTHON_CONFIG_WHICH),)
	PYTHON2_INCLUDE=$(shell python-config --includes)
	PYTHON2_LIB=$(shell python-config --ldflags)
endif

# look for numpy include directory
#NUMPY_INCLUDE = $(shell python $(CROW_DIR)/scripts/find_numpy_include.py)

ifeq ($(PYTHON3_HELLO),HELLO)
	PYTHON_INCLUDE = $(shell $(CROW_DIR)/scripts/find_flags.py include) #-DPy_LIMITED_API
	PYTHON_LIB = $(shell $(CROW_DIR)/scripts/find_flags.py library) #-DPy_LIMITED_API
ifeq ($(findstring SWIG Version 2,$(SWIG_VERSION)),)
	CONTROL_MODULES =
	PYTHON_MODULES =
else
	SWIG_PY_FLAGS=-py3
	CONTROL_MODULES = $(CROW_DIR)/control_modules/_distribution1D.so $(CROW_DIR)/control_modules/_crowtools.so
	PYTHON_MODULES = $(CROW_DIR)/python_modules/_distribution1Dpy2.so $(CROW_DIR)/python_modules/_distribution1Dpy3.so $(CROW_DIR)/python_modules/_interpolationNDpy2.so $(CROW_DIR)/python_modules/_interpolationNDpy3.so
endif #Have SWIG

else #no Python3
ifeq ($(PYTHON2_HELLO),HELLO)
ifeq ($(PYTHON_CONFIG_WHICH),)
	PYTHON_INCLUDE = -DNO_PYTHON3_FOR_YOU
	PYTHON_LIB = -DNO_PYTHON3_FOR_YOU
	CONTROL_MODULES =
	PYTHON_MODULES =
else #Python 2 and Python config found but not Python 3
	PYTHON_INCLUDE=$(PYTHON2_INCLUDE)
	PYTHON_LIB=$(PYTHON2_LIB)
	#CONTROL_MODULES=
	SWIG_PY_FLAGS=
	PYTHON_MODULES = $(CROW_DIR)/python_modules/_distribution1Dpy2.so $(CROW_DIR)/python_modules/_interpolationNDpy2.so
	CONTROL_MODULES=$(CROW_DIR)/control_modules/_distribution1D.so $(CROW_DIR)/control_modules/_crowtools.so
endif
else
#Python3 and Python2 not found.
	PYTHON_INCLUDE = -DNO_PYTHON3_FOR_YOU
	PYTHON_LIB = -DNO_PYTHON3_FOR_YOU
	CONTROL_MODULES =
	PYTHON_MODULES =
endif
endif

CROW_LIB_INCLUDE_DIR := $(CROW_DIR)/contrib/include

ifeq  ($(UNAME),Darwin)
crow_shared_ext := dylib
else
crow_shared_ext := so
endif

HAS_DYNAMIC := $(shell $(libmesh_LIBTOOL) --config | grep build_libtool_libs | cut -d'=' -f2 )

ifeq ($(HAS_DYNAMIC),no)
ifdef CONTROL_MODULES
$(warning CROW modules must be compiled with shared libmesh libraries)
	CONTROL_MODULES =
endif
endif
