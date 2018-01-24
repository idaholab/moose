CXX ?= g++

PYTHONPREFIX ?= `python-config --prefix`
PYTHONCFLAGS ?= `python-config --cflags`
PYTHONLDFLAGS ?= `python-config --ldflags`

hit: main.cc parse.cc lex.cc lex.h parse.h
	$(CXX) -std=c++11 -g $(CXXFLAGS) $< parse.cc lex.cc -o $@

rewrite: rewrite.cc parse.cc lex.cc lex.h parse.h
	$(CXX) -std=c++11 -g $(CXXFLAGS) $< parse.cc lex.cc -o $@

bindings: hit.so

hit.so: parse.cc lex.cc hit.cpp
	$(CXX) -std=c++11 -w -fPIC -lstdc++ -shared -L$(PYTHONPREFIX)/lib $(PYTHONCFLAGS) $(PYTHONLDFLAGS)  $^ -o $@

hit.cpp: hit.pyx chit.pxd
	cython --cplus $<

.PRECIOUS: hit.cpp

.PHONY: clean bindings

clean:
	rm -f hit hit.so
