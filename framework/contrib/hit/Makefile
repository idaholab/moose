hit_CXX := $(libmesh_CXX)

ifeq ($(hit_CXX),)
  hit_CXX := $(CXX)
endif

ifeq ($(hit_CXX),)
  hit_CXX := g++
endif

UNAME := $(shell uname)

pyconfig := python3-config
ifeq (, $(shell which $(pyconfig) 2>/dev/null))
	pyconfig := python-config
endif
cython := cython3
ifeq (, $(shell which $(cython) 2>/dev/null))
	cython := 'cython -3'
endif

ifeq ($(UNAME), Darwin)
	DYNAMIC_LOOKUP := -undefined dynamic_lookup
else
	DYNAMIC_LOOKUP :=
endif

$(info Building hit for python with $(pyconfig))

hit_srcdir         := src/hit
hit_srcfiles       := $(hit_srcdir)/parse.cc $(hit_srcdir)/lex.cc $(hit_srcdir)/braceexpr.cc
hit_includeflags   := -Iinclude

PYTHONPREFIX ?= `$(pyconfig) --prefix`
PYTHONCFLAGS ?= `$(pyconfig) --cflags`
HITCPP := $(hit_srcdir)/hit.cpp

WASP_DIR           ?= $(abspath ../wasp/install)

# find the wasp libraries
lib_suffix := so
ifeq ($(shell uname -s),Darwin)
	lib_suffix := dylib
	wasp_LIBS         := $(shell find -E $(WASP_DIR)/lib -regex ".*/lib[a-z]+.$(lib_suffix)")
else
	wasp_LIBS         := $(wildcard $(WASP_DIR)/lib/libwasp*$(lib_suffix))
endif
wasp_LIBS         := $(notdir $(wasp_LIBS))
wasp_LIBS         := $(patsubst %.$(lib_suffix),%,$(wasp_LIBS))
wasp_LIBS         := $(patsubst lib%,-l%,$(wasp_LIBS))
ifeq ($(wasp_LIBS),)
  $(error WASP does not seem to be available. Make sure to either run scripts/update_and_rebuild_wasp.sh in your MOOSE directory, or set WASP_DIR to a valid WASP install)
endif

wasp_CXXFLAGS  += -I$(WASP_DIR)/include
wasp_LDFLAGS   += -Wl,-rpath,$(WASP_DIR)/lib -L$(WASP_DIR)/lib $(wasp_LIBS)

hit: $(hit_srcdir)/main.cc $(hit_srcfiles) include/hit/braceexpr.h include/hit/lex.h include/hit/parse.h
	$(hit_CXX) -std=c++17 $(wasp_CXXFLAGS) $(hit_includeflags) -g $(CXXFLAGS) $< $(hit_srcfiles) -o $@ $(wasp_LDFLAGS)

bindings: hit.so

hit.so: $(hit_srcfiles)
	$(hit_CXX) -std=c++17 $(wasp_CXXFLAGS) $(hit_includeflags) -w -fPIC -lstdc++ -shared -L$(PYTHONPREFIX)/lib $(PYTHONCFLAGS) $(DYNAMIC_LOOKUP) $^ $(HITCPP) -o $@ $(wasp_LDFLAGS)

$(HITCPP): hit.pyx chit.pxd
	$(cython) -o $@ --cplus $<

.PRECIOUS: $(HITCPP)

.PHONY: clean bindings

clean:
	rm -f hit hit.so
