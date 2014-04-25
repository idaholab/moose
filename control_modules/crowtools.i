%module raventools
%{
#include "RavenToolsContainer.h"
%}
%include "RavenToolsContainer.h"


/*
IT IS AN EXAMPLEEEEEEEEEEEE
IT IS COMMENTEDDDDDDDD
swig -c++ -python -py3 -Iinclude/tools/ -I../moose/include/utils/ control_modules/raventools.i 
g++ -fPIC -c src/tools/*.C control_modules/raventools_wrap.cxx -Iinclude/tools/ -I/usr/include/python3.2mu/
g++ -shared *.o -o control_modules/_raventools.so
PYTHONPATH=control_modules/ python3

#rm -f *.o *.so raventools.py

swig -c++ -python -py3 -Iinclude/tools control_modules/raventools.i
g++ -fPIC -c src/tools/*.C control_modules/raventools_wrap.cxx -Iinclude/tools/ -I/usr/include/python3.2mu/ -Iinclude/utilities -Iinclude/base
g++ -shared *.o -o control_modules/_raventools.so

*/


