CXX ?= g++
UNAME := $(shell uname)

pyconfig := python3-config
ifeq (, $(shell which $(pyconfig) 2>/dev/null))
	pyconfig := python-config
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

hit: main.cc parse.cc lex.cc braceexpr.cc braceexpr.h lex.h parse.h
	$(CXX) -std=c++17 -g $(CXXFLAGS) $< parse.cc lex.cc braceexpr.cc -o $@

bindings: hit.so

hit.so: parse.cc lex.cc braceexpr.cc
	$(CXX) -std=c++17 -w -fPIC -lstdc++ -shared -L$(PYTHONPREFIX)/lib $(PYTHONCFLAGS) $(DYNAMIC_LOOKUP) $^ $(HITCPP) -o $@

$(HITCPP): hit.pyx chit.pxd
	cython -3 -o $@ --cplus $<

.PRECIOUS: $(HITCPP)

.PHONY: clean bindings

clean:
	rm -f hit hit.so
