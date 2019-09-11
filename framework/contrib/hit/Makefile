CXX ?= g++
PYTHON_VERSION := $(shell python -c 'import sys;print(sys.version_info[0])')

ifeq ($(PYTHON_VERSION), 2)
	pyconfig := python-config
else
	pyconfig := python3-config
	ifeq (, $(shell which python3-config 2>/dev/null))
		pyconfig := python-config
	endif
endif

$(info Building hit for python $(PYTHON_VERSION) with $(pyconfig))

PYTHONPREFIX ?= `$(pyconfig) --prefix`
PYTHONCFLAGS ?= `$(pyconfig) --cflags`
PYTHONLDFLAGS ?= `$(pyconfig) --ldflags`
HITCPP := hit$(PYTHON_VERSION).cpp

hit: main.cc parse.cc lex.cc braceexpr.cc braceexpr.h lex.h parse.h
	$(CXX) -std=c++11 -g $(CXXFLAGS) $< parse.cc lex.cc braceexpr.cc -o $@

rewrite: rewrite.cc parse.cc lex.cc lex.h parse.h
	$(CXX) -std=c++11 -g $(CXXFLAGS) $< parse.cc lex.cc -o $@

bindings: hit.so

hit.so: parse.cc lex.cc braceexpr.cc $(HITCPP)
	$(CXX) -std=c++11 -w -fPIC -lstdc++ -shared -L$(PYTHONPREFIX)/lib $(PYTHONCFLAGS) $(PYTHONLDFLAGS) $^ -o $@

$(HITCPP): hit.pyx chit.pxd
	cython -$(PYTHON_VERSION) -o $@ --cplus $<

.PRECIOUS: $(HITCPP)

.PHONY: clean bindings

clean:
	rm -f hit hit.so
