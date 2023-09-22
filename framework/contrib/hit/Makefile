CXX ?= g++
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

PYTHONPREFIX ?= `$(pyconfig) --prefix`
PYTHONCFLAGS ?= `$(pyconfig) --cflags`
HITCPP := hit.cpp

WASP_DIR           ?= $(abspath ../wasp/install)

# find the wasp libraries
lib_suffix := so
ifeq ($(shell uname -s),Darwin)
	lib_suffix := dylib
endif
wasp_LIBS         := $(wildcard $(WASP_DIR)/lib/libwasp*$(lib_suffix))
wasp_LIBS         := $(notdir $(wasp_LIBS))
wasp_LIBS         := $(patsubst %.$(lib_suffix),%,$(wasp_LIBS))
wasp_LIBS         := $(patsubst lib%,-l%,$(wasp_LIBS))
ifeq ($(wasp_LIBS),)
  $(error WASP does not seem to be available. Make sure to either run scripts/update_and_rebuild_wasp.sh in your MOOSE directory, or set WASP_DIR to a valid WASP install)
endif

wasp_CXXFLAGS  += -DWASP_ENABLED -I$(WASP_DIR)/include
wasp_LDFLAGS   += -Wl,-rpath,$(WASP_DIR)/lib -L$(WASP_DIR)/lib $(wasp_LIBS)

hit: main.cc parse.cc lex.cc braceexpr.cc braceexpr.h lex.h parse.h
	$(CXX) -std=c++17 $(wasp_CXXFLAGS) -g $(CXXFLAGS) $(wasp_LDFLAGS) $< parse.cc lex.cc braceexpr.cc -o $@

bindings: hit.so

hit.so: parse.cc lex.cc braceexpr.cc
	$(CXX) -std=c++17 $(wasp_CXXFLAGS) -w -fPIC -lstdc++ -shared $(wasp_LDFLAGS) -L$(PYTHONPREFIX)/lib $(PYTHONCFLAGS) $(DYNAMIC_LOOKUP) $^ $(HITCPP) -o $@

$(HITCPP): hit.pyx chit.pxd
	$(cython) -o $@ --cplus $<

.PRECIOUS: $(HITCPP)

.PHONY: clean bindings

clean:
	rm -f hit hit.so
