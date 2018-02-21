CXX ?= g++

hit: main.cc parse.cc lex.cc lex.h parse.h
	$(CXX) -std=c++11 -g $(CXXFLAGS) $< parse.cc lex.cc -o $@

rewrite: rewrite.cc parse.cc lex.cc lex.h parse.h
	$(CXX) -std=c++11 -g $(CXXFLAGS) $< parse.cc lex.cc -o $@

bindings: hit.so

hit.so: parse.cc lex.cc hit.cpp
	python setup.py build_ext --inplace $(PYTHONCOMPILERFLAG)

hit.cpp: hit.pyx chit.pxd
	cython --cplus $<

.PRECIOUS: hit.cpp

.PHONY: clean bindings

clean:
	rm -f hit hit.so
