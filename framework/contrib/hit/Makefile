CXX ?= g++

# some systems have python2 but no python2-config command - fall back to python-config for them
pyconfig := python3-config
ifeq (, $(shell which python3-config 2>/dev/null))
  pyconfig := python-config
endif

$(info $(pyconfig))

PYTHONPREFIX ?= `$(pyconfig) --prefix`
PYTHONCFLAGS ?= `$(pyconfig) --cflags`
PYTHONLDFLAGS ?= `$(pyconfig) --ldflags`

hit: main.cc parse.cc lex.cc braceexpr.cc braceexpr.h lex.h parse.h
	$(CXX) -std=c++11 -g $(CXXFLAGS) $< parse.cc lex.cc braceexpr.cc -o $@

rewrite: rewrite.cc parse.cc lex.cc lex.h parse.h
	$(CXX) -std=c++11 -g $(CXXFLAGS) $< parse.cc lex.cc -o $@

bindings: hit.so

hit.so: parse.cc lex.cc braceexpr.cc hit.cpp
	$(CXX) -std=c++11 -w -fPIC -lstdc++ -shared -L$(PYTHONPREFIX)/lib $(PYTHONCFLAGS) $(PYTHONLDFLAGS) $^ -o $@

hit.cpp: hit.pyx chit.pxd
	cython -3 --cplus $<

.PRECIOUS: hit.cpp

.PHONY: clean bindings

clean:
	rm -f hit hit.so
