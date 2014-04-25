%module distribution1D
%{
#include "distribution.h"
#include "DistributionContainer.h"
#include "distribution_1D.h"
#include "distribution_base_ND.h"
%}
%include "std_vector.i"
%include "distribution.h"
%include "DistributionContainer.h"
%include "distribution_1D.h"
%include "distribution_base_ND.h"

namespace std {
   %template(vectord_cxx) vector<double>;
};


 /*
swig -c++ -python -py3 -Iinclude/distributions/ -Iinclude/base/ -I$HOME/raven_libs/pylibs/lib/python2.7/site-packages -I../moose/include/utils/ control_modules/distribution1D.i 
g++ -fPIC -c src/distributions/*.C control_modules/distribution1D_wrap.cxx -Iinclude/distributions/ -Iinclude/utilities/ -I/usr/include/python3.2mu/
g++ -shared *.o -o control_modules/_distribution1D.so
PYTHONPATH=control_modules/ python3
import distribution1D
test1 = distribution1D.distribution_1D(1, -3.0, 2.0,  1.0, 1.0)
test1.randGen()

distcont = distribution1D.DistributionContainer.Instance()
distcont.constructDistributionContainer(distribution1D.str_to_string_p("a_dist"),distribution1D.NORMAL_DISTRIBUTION,-1.0,1.0,0.0,1.0)
distcont.randGen(distribution1D.str_to_string_p("a_dist"))

#rm -f *.o *.so distribution1D.py

swig -c++ -python -py3 -Iinclude/tools control_modules/raventools.i
g++ -fPIC -c src/tools/*.C control_modules/raventools_wrap.cxx -Iinclude/tools/ -I/usr/include/python3.2mu/ -Iinclude/utilities
g++ -shared *.o -o control_modules/_raventools.so

  */
