CXX ?= g++

# some systems have python2 but no python2-config command - fall back to python-config for them
pyconfig := python2-config
ifeq (, $(shell which python2-config))
  pyconfig := python-config
endif

PYTHONPREFIX ?= `$(pyconfig) --prefix`
PYTHONCFLAGS ?= `$(pyconfig) --cflags`
PYTHONLDFLAGS ?= `$(pyconfig) --ldflags`

hit: main.cc parse.cc lex.cc lex.h parse.h
	$(CXX) -std=c++11 -g $(CXXFLAGS) $< parse.cc lex.cc -o $@

rewrite: rewrite.cc parse.cc lex.cc lex.h parse.h
	$(CXX) -std=c++11 -g $(CXXFLAGS) $< parse.cc lex.cc -o $@

bindings: hit.so

hit.so: parse.cc lex.cc hit.cpp
	$(CXX) -std=c++11 -w -fPIC -lstdc++ -shared -L$(PYTHONPREFIX)/lib $(PYTHONCFLAGS) $(PYTHONLDFLAGS) $^ -o $@

hit.cpp: hit.pyx chit.pxd
	cython --cplus $<

.PRECIOUS: hit.cpp

.PHONY: clean bindings

clean:
	rm -f hit hit.so
