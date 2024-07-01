//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MooseTypes.h"
#include "PermutationTensor.h"
#include "RankFourTensor.h"
#include "ElasticityTensorTools.h"

#include "libmesh/vector_value.h"

namespace ElasticityTensorTools
{

Real
elasticJacobian(const RankFourTensor & r4t,
                unsigned int i,
                unsigned int k,
                const RealGradient & grad_test,
                const RealGradient & grad_phi)
{
  // d(stress_ij*d(test)/dx_j)/du_k = d(C_ijmn*du_m/dx_n dtest/dx_j)/du_k (which is nonzero for m ==
  // k)

  const Real gt0 = grad_test(0);
  const Real gt1 = grad_test(1);
  const Real gt2 = grad_test(2);
  const Real gp0 = grad_phi(0);
  const Real gp1 = grad_phi(1);
  const Real gp2 = grad_phi(2);

  // clang-format off
  // This is the algorithm that is unrolled below:
  //
  //    Real sum = 0.0;
  //    for (const auto j: make_range(Moose::dim))
  //      for (const auto l: make_range(Moose::dim))
  //        sum += r4t(i, j, k, l) * grad_phi(l) * grad_test(j);
  //    return sum;

  return
     (
         r4t(i,0,k,0) * gp0
       + r4t(i,0,k,1) * gp1
       + r4t(i,0,k,2) * gp2
     ) * gt0
     +
     (
         r4t(i,1,k,0) * gp0
       + r4t(i,1,k,1) * gp1
       + r4t(i,1,k,2) * gp2
     ) * gt1
     +
     (
         r4t(i,2,k,0) * gp0
       + r4t(i,2,k,1) * gp1
       + r4t(i,2,k,2) * gp2
     ) * gt2;
  // clang-format on
}

Real
elasticJacobianWC(const RankFourTensor & r4t,
                  unsigned int i,
                  unsigned int k,
                  const RealGradient & grad_test,
                  Real phi)
{
  // d(stress_ij*d(test)/dx_j)/dw_k = d(C_ijmn*eps_mnp*w_p*dtest/dx_j)/dw_k (only nonzero for p ==
  // k)
  Real sum = 0.0;
  for (const auto j : make_range(Moose::dim))
    for (const auto m : make_range(Moose::dim))
      for (const auto n : make_range(Moose::dim))
        sum += r4t(i, j, m, n) * PermutationTensor::eps(m, n, k) * grad_test(j);
  return sum * phi;
}

Real
momentJacobian(const RankFourTensor & r4t,
               unsigned int i,
               unsigned int k,
               Real test,
               const RealGradient & grad_phi)
{
  // Jacobian entry: d(eps_ijm*stress_jm*test)/du_k = d(eps_ijm*C_jmln*du_l/dx_n*test)/du_k (only
  // nonzero for l == k)
  Real sum = 0.0;
  for (const auto j : make_range(Moose::dim))
    for (const auto m : make_range(Moose::dim))
      for (const auto n : make_range(Moose::dim))
        sum += PermutationTensor::eps(i, j, m) * r4t(j, m, k, n) * grad_phi(n);
  return test * sum;
}

Real
momentJacobianWC(const RankFourTensor & r4t, unsigned int i, unsigned int k, Real test, Real phi)
{
  // Jacobian entry: d(eps_ijm*stress_jm*test)/dw_k = d(eps_ijm*C_jmln*eps_lnp*w_p*test)/dw_k (only
  // nonzero for p ==k)
  Real sum = 0.0;
  for (const auto j : make_range(Moose::dim))
    for (const auto l : make_range(Moose::dim))
      for (const auto m : make_range(Moose::dim))
        for (const auto n : make_range(Moose::dim))
          sum +=
              PermutationTensor::eps(i, j, m) * r4t(j, m, l, n) * PermutationTensor::eps(l, n, k);

  return test * phi * sum;
}

void
toVoigtNotationIndexConversion(int k, int & a, int & b)
{
  if (k < 3 && k >= 0)
    a = b = k;
  else if (k == 3)
  {
    a = 1;
    b = 2;
  }
  else if (k == 4)
  {
    a = 0;
    b = 2;
  }
  else if (k == 5)
  {
    a = 0;
    b = 1;
  }
  else
    mooseError("\nIndex out of bound while converting from tensor to voigt notation in "
               "toVoigtNotationIndexConversion");
}

// MOOSE uses the stress tensor with components ordered as (11, 22, 33, 12, 23, 13) in voigt form.
// This is in conflict with the voigt notation used in literature which has components ordered as
// (11, 22, 33, 23, 13, 12). Whenever an operation involving the voigt forms of stress and
// elasticity tensor has to be performed the voigt form of the elasticity tensor should be built
// taking into account this difference in component-ordering for stress. The
// toMooseVoigtNotationIndexConversion function facilitates that.

void
toMooseVoigtNotationIndexConversion(int k, int & a, int & b)
{
  if (k < 3 && k >= 0)
    a = b = k;
  else if (k == 3)
  {
    a = 0;
    b = 1;
  }
  else if (k == 4)
  {
    a = 1;
    b = 2;
  }
  else if (k == 5)
  {
    a = 0;
    b = 2;
  }
  else
    mooseError("\nIndex out of bound while converting from tensor to MOOSE voigt notation in "
               "toMooseVoigtNotationIndexConversion");
}
}
