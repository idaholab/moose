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
   %template(vectori_cxx) vector<int>;
};

