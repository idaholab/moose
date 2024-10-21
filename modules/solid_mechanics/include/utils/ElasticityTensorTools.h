//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "RankFourTensorForward.h"

namespace libMesh
{
template <typename>
class VectorValue;
typedef VectorValue<Real> RealGradient;
}
using libMesh::RealGradient;

namespace ElasticityTensorTools
{

/**
 * This is used for the standard kernel stress_ij*d(test)/dx_j, when varied wrt u_k
 * Jacobian entry: d(stress_ij*d(test)/dx_j)/du_k = d(C_ijmn*du_m/dx_n*dtest/dx_j)/du_k
 */
Real elasticJacobian(const RankFourTensor & r4t,
                     unsigned int i,
                     unsigned int k,
                     const RealGradient & grad_test,
                     const RealGradient & grad_phi);

/**
 * This is used for the standard kernel stress_ij*d(test)/dx_j, when varied wrt w_k (the cosserat
 * rotation)
 * Jacobian entry: d(stress_ij*d(test)/dx_j)/dw_k = d(C_ijmn*eps_mnp*w_p*dtest/dx_j)/dw_k
 */
Real elasticJacobianWC(const RankFourTensor & r4t,
                       unsigned int i,
                       unsigned int k,
                       const RealGradient & grad_test,
                       Real phi);

/**
 * This is used for the moment-balancing kernel eps_ijk*stress_jk*test, when varied wrt u_k
 * Jacobian entry: d(eps_ijm*stress_jm*test)/du_k = d(eps_ijm*C_jmln*du_l/dx_n*test)/du_k
 */
Real momentJacobian(const RankFourTensor & r4t,
                    unsigned int i,
                    unsigned int k,
                    Real test,
                    const RealGradient & grad_phi);

/**
 * This is used for the moment-balancing kernel eps_ijk*stress_jk*test, when varied wrt w_k (the
 * cosserat rotation)
 * Jacobian entry: d(eps_ijm*stress_jm*test)/dw_k = d(eps_ijm*C_jmln*eps_lnp*w_p*test)/dw_k
 */
Real
momentJacobianWC(const RankFourTensor & r4t, unsigned int i, unsigned int k, Real test, Real phi);

/**
 * Get the shear modulus for an isotropic elasticity tensor
 * param elasticity_tensor the tensor (must be isotropic, but not checked for efficiency)
 */
template <typename T>
T
getIsotropicShearModulus(const RankFourTensorTempl<T> & elasticity_tensor)
{
  return elasticity_tensor(0, 1, 0, 1);
}

/**
 * Get the bulk modulus for an isotropic elasticity tensor
 * param elasticity_tensor the tensor (must be isotropic, but not checked for efficiency)
 */
template <typename T>
T
getIsotropicBulkModulus(const RankFourTensorTempl<T> & elasticity_tensor)
{
  const T shear_modulus = getIsotropicShearModulus(elasticity_tensor);
  // dilatational modulus is defined as lambda plus two mu
  const T dilatational_modulus = elasticity_tensor(0, 0, 0, 0);
  const T lambda = dilatational_modulus - 2.0 * shear_modulus;
  const T bulk_modulus = lambda + 2.0 * shear_modulus / 3.0;
  return bulk_modulus;
}

/**
 * Get the Young's modulus for an isotropic elasticity tensor
 * param elasticity_tensor the tensor (must be isotropic, but not checked for efficiency)
 */
template <typename T>
T
getIsotropicYoungsModulus(const RankFourTensorTempl<T> & elasticity_tensor)
{
  const T shear_modulus = getIsotropicShearModulus(elasticity_tensor);
  // dilatational modulus is defined as lambda plus two mu
  const T dilatational_modulus = elasticity_tensor(0, 0, 0, 0);
  const T lambda = dilatational_modulus - 2.0 * shear_modulus;
  const T youngs_modulus =
      shear_modulus * (3.0 * lambda + 2.0 * shear_modulus) / (lambda + shear_modulus);
  return youngs_modulus;
}

/**
 * Get the Poisson's modulus for an isotropic elasticity tensor
 * param elasticity_tensor the tensor (must be isotropic, but not checked for efficiency)
 */
template <typename T>
T
getIsotropicPoissonsRatio(const RankFourTensorTempl<T> & elasticity_tensor)
{
  const T poissons_ratio = elasticity_tensor(1, 1, 0, 0) /
                           (elasticity_tensor(1, 1, 1, 1) + elasticity_tensor(1, 1, 0, 0));
  return poissons_ratio;
}

void toVoigtNotationIndexConversion(int, int &, int &);

template <bool is_ad>
void
toVoigtNotation(GenericDenseMatrix<is_ad> & voigt_matrix,
                const GenericRankFourTensor<is_ad> & tensor)
{
  std::vector<int> index_vector = {0, 1, 2, 3, 4, 5};
  int a = 0;
  int b = 0;
  int c = 0;
  int d = 0;
  for (int i : index_vector)
    for (int j : index_vector)
    {
      toVoigtNotationIndexConversion(i, a, b);
      toVoigtNotationIndexConversion(j, c, d);
      voigt_matrix(i, j) = tensor(a, b, c, d);
    }
}

void toMooseVoigtNotationIndexConversion(int, int &, int &);

template <bool is_ad>
void
toMooseVoigtNotation(GenericDenseMatrix<is_ad> & voigt_matrix,
                     const GenericRankFourTensor<is_ad> & tensor)
{
  static std::vector<int> index_vector = {0, 1, 2, 3, 4, 5};
  int a = 0;
  int b = 0;
  int c = 0;
  int d = 0;
  for (int i : index_vector)
    for (int j : index_vector)
    {
      toMooseVoigtNotationIndexConversion(i, a, b);
      toMooseVoigtNotationIndexConversion(j, c, d);
      voigt_matrix(i, j) = tensor(a, b, c, d);
    }
}
}
