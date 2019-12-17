CXX ?= g++
PYTHON_VERSION := $(shell python -c 'import sys;print(sys.version_info[0])')
UNAME := $(shell uname)

ifeq ($(PYTHON_VERSION), 2)
	pyconfig := python2-config
else
	pyconfig := python3-config
endif

ifeq (, $(shell which $(pyconfig) 2>/dev/null))
	pyconfig := python-config
endif

ifeq ($(UNAME), Darwin)
	DYNAMIC_LOOKUP := -undefined dynamic_lookup
else
	DYNAMIC_LOOKUP := ""
endif

$(info Building hit for python $(PYTHON_VERSION) with $(pyconfig))

PYTHONPREFIX ?= `$(pyconfig) --prefix`
PYTHONCFLAGS ?= `$(pyconfig) --cflags`
HITCPP := hit$(PYTHON_VERSION).cpp

hit: main.cc parse.cc lex.cc braceexpr.cc braceexpr.h lex.h parse.h
	$(CXX) -std=c++11 -g $(CXXFLAGS) $< parse.cc lex.cc braceexpr.cc -o $@

rewrite: rewrite.cc parse.cc lex.cc lex.h parse.h
	$(CXX) -std=c++11 -g $(CXXFLAGS) $< parse.cc lex.cc -o $@

bindings: hit.so

hit.so: parse.cc lex.cc braceexpr.cc $(HITCPP)
	$(CXX) -std=c++11 -w -fPIC -lstdc++ -shared -L$(PYTHONPREFIX)/lib $(PYTHONCFLAGS) $(DYNAMIC_LOOKUP) $^ -o $@

$(HITCPP): hit.pyx chit.pxd
	cython -$(PYTHON_VERSION) -o $@ --cplus $<

.PRECIOUS: $(HITCPP)

.PHONY: clean bindings

clean:
	rm -f hit hit.so
