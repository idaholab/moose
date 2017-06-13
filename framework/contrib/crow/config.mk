CROW_DIR ?= $(ROOT_DIR)/crow

SWIG_VERSION := $(shell swig -version 2>/dev/null)
PYTHON_WHICH := $(shell which python 2>/dev/null)
ifeq ($(CROW_USE_PYTHON3),TRUE)
	PYTHON3_WHICH := $(shell which python3 2>/dev/null)
endif

ifneq ($(findstring SWIG Version 2,$(SWIG_VERSION)),)
	HAS_SWIG := true
else
ifneq ($(findstring SWIG Version 3,$(SWIG_VERSION)),)
	HAS_SWIG := true
else
	HAS_SWIG := false
$(warning Swig is required to build crow)
endif
endif


UNAME := $(shell uname)

ifneq ($(PYTHON3_WHICH),)
	SWIG_PY_FLAGS=-py3
	PYTHON_MODULES = $(CROW_DIR)/install/crow_modules/_distribution1Dpy2.so $(CROW_DIR)/install/crow_modules/_interpolationNDpy2.so $(CROW_DIR)/install/crow_modules/_distribution1Dpy3.so $(CROW_DIR)/install/crow_modules/_interpolationNDpy3.so

else #no Python3

ifneq ($(PYTHON_WHICH),)
#Python 2 found but not Python 3
	SWIG_PY_FLAGS=
	PYTHON_MODULES = $(CROW_DIR)/install/crow_modules/_distribution1Dpy2.so $(CROW_DIR)/install/crow_modules/_interpolationNDpy2.so
else #No python3 config or python2 config
	PYTHON_MODULES =
$(warning python not found)
endif

endif

ifeq ($(coverage),true)
        COVERAGE_COMPILE_EXTRA = -fprofile-arcs -ftest-coverage
	ifeq (,$(findstring clang++,$(cxx_compiler)))
		COVERAGE_LINK_EXTRA = -lgcov
	else
                COVERAGE_LINK_EXTRA =
	endif
else
        COVERAGE_COMPILE_EXTRA =
        COVERAGE_LINK_EXTRA =
endif

CROW_LIB_INCLUDE_DIR := $(CROW_DIR)/contrib/include
EIGEN_INCLUDE := $(shell $(CROW_DIR)/scripts/find_eigen.py)

python_crow_modules :: $(PYTHON_MODULES)

ifeq  ($(UNAME),Darwin)
crow_shared_ext := dylib
else
crow_shared_ext := so
endif


