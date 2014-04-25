%module interpolationNDpy2
%{
#include "ND_Interpolation_Functions.h"
#define SWIG_FILE_WITH_INIT
#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
%}
%include "std_vector.i"
%include "ND_Interpolation_Functions.h"

namespace std {
   %template(vectd) vector<double>;
   %template(vectd2d) vector< vector<double> >;
   %template(vecti) vector<int>;
   %template(vecti2d) vector< vector<int> >;
};

