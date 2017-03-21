/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "MooseTypes.h"
#include "RankFourTensor.h"

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
  Real sum = 0.0;
  for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
    for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
      sum += r4t(i, j, k, l) * grad_phi(l) * grad_test(j);
  return sum;
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
  for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
    for (unsigned int m = 0; m < LIBMESH_DIM; ++m)
      for (unsigned int n = 0; n < LIBMESH_DIM; ++n)
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
  for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
    for (unsigned int m = 0; m < LIBMESH_DIM; ++m)
      for (unsigned int n = 0; n < LIBMESH_DIM; ++n)
        sum += PermutationTensor::eps(i, j, m) * r4t(j, m, k, n) * grad_phi(n);
  return test * sum;
}

Real
momentJacobianWC(const RankFourTensor & r4t, unsigned int i, unsigned int k, Real test, Real phi)
{
  // Jacobian entry: d(eps_ijm*stress_jm*test)/dw_k = d(eps_ijm*C_jmln*eps_lnp*w_p*test)/dw_k (only
  // nonzero for p ==k)
  Real sum = 0.0;
  for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
    for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
      for (unsigned int m = 0; m < LIBMESH_DIM; ++m)
        for (unsigned int n = 0; n < LIBMESH_DIM; ++n)
          sum +=
              PermutationTensor::eps(i, j, m) * r4t(j, m, l, n) * PermutationTensor::eps(l, n, k);

  return test * phi * sum;
}
}
