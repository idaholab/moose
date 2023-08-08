#include "Python.h"

#ifndef Py_PYTHON_H
#error Python headers needed to compile C extensions, please install development version of Python.
#elif PY_VERSION_HEX < 0x030b0000
#include "hitpre311.cpp"
#else
#include "hit311.cpp"
#endif
