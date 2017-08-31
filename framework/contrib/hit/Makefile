
CXX ?= g++

hit: main.cc parse.cc lex.cc lex.h parse.h
	$(CXX) -std=c++11 -g $< parse.cc lex.cc -o $@

clean:
	rm -f hit
