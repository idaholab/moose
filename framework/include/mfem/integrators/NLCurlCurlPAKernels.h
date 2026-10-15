//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "mfem/fem/integ/bilininteg_hcurl_kernels.hpp"
#include "libmesh/restore_warnings.h"

/*
All this header does is declare the functions that will hold
the calls to mfem kernels.

Note that we deliberately do not use the #ifdef MOOSE_MFEM_ENABLED macro
here. We don't include any moose headers (eg MooseConfig.h) so we don't
include the definition for this macro.
*/

namespace Moose::MFEM
{

// The params are:
// const int Q1D - the number of quadpoints in each dimension
// (we skip coeff_dim, as we expect the functions to be scalar functions)
// const int NE - the number of elements
// const Array<real_t> & w, the quadrature weights - get these from the integration rule
// const Vector &j - the jacobian matrices in the form of a flat vector. Its length is
//    9 * number of elements * number of quadpoints, i.e. one 3x3 matrix for each quadpoint
//    in each element
// const Vector& c - this is the curl, evaluated at each qpoint
// Vector & k_coeff  - reference to the vector of k(s) evaluated at each quadpoint. length is
//    one per quadpoint per element
// Vector & dk_coeff - ditto for s * k'(s)
// Vector & op - this is the diagonal operator
void NLCurlCurlGradPASetup(const int Q1D,
                           const int NE,
                           const mfem::Array<mfem::real_t> & w,
                           const mfem::Vector & j,
                           const mfem::Vector & c,
                           mfem::Vector & k_coeff,
                           mfem::Vector & dk_coeff,
                           mfem::Vector & op,
                           mfem::real_t curlu_zero_tol);

// Same, but creates the op used for AddMultPA
// const int Q1D - the number of quadpoints in each dimension
// (we skip coeff_dim, as we expect the functions to be scalar functions)
// const int NE - the number of elements
// const Array<real_t> & w, the quadrature weights - get these from the integration rule
// const Vector &j - the jacobian matrices in the form of a flat vector. Its length is
//    9 * number of elements * number of quadpoints, i.e. one 3x3 matrix for each quadpoint
//    in each element
// Vector & k_coeff  - reference to the vector of k(s) evaluated at each quadpoint. length is
//    one per quadpoint per element
// Vector & op - this is the diagonal operator
void NLCurlCurlPASetup(const int Q1D,
                       const int NE,
                       const mfem::Array<mfem::real_t> & w,
                       const mfem::Vector & j,
                       mfem::Vector & k_coeff,
                       mfem::Vector & op);

} // namespace Moose::MFEM
