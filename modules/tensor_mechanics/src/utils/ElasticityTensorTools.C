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
  //    for (unsigned int j = 0; j < LIBMESH_DIM; ++j)
  //      for (unsigned int l = 0; l < LIBMESH_DIM; ++l)
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

Real
getIsotropicShearModulus(const RankFourTensor & elasticity_tensor)
{
  const Real shear_modulus = elasticity_tensor(0, 1, 0, 1);
  return shear_modulus;
}

Real
getIsotropicBulkModulus(const RankFourTensor & elasticity_tensor)
{
  const Real shear_modulus = getIsotropicShearModulus(elasticity_tensor);
  // dilatational modulus is defined as lambda plus two mu
  const Real dilatational_modulus = elasticity_tensor(0, 0, 0, 0);
  const Real lambda = dilatational_modulus - 2.0 * shear_modulus;
  const Real bulk_modulus = lambda + 2.0 * shear_modulus / 3.0;
  return bulk_modulus;
}

Real
getIsotropicYoungsModulus(const RankFourTensor & elasticity_tensor)
{
  const Real shear_modulus = getIsotropicShearModulus(elasticity_tensor);
  // dilatational modulus is defined as lambda plus two mu
  const Real dilatational_modulus = elasticity_tensor(0, 0, 0, 0);
  const Real lambda = dilatational_modulus - 2.0 * shear_modulus;
  const Real youngs_modulus =
      shear_modulus * (3.0 * lambda + 2.0 * shear_modulus) / (lambda + shear_modulus);
  return youngs_modulus;
}

Real
getIsotropicPoissonsRatio(const RankFourTensor & elasticity_tensor)
{
  const Real poissons_ratio = elasticity_tensor(1, 1, 0, 0) /
                              (elasticity_tensor(1, 1, 1, 1) + elasticity_tensor(1, 1, 0, 0));
  return poissons_ratio;
}
}
