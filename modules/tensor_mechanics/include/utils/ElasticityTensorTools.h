//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef ELASTICITYTENSORTOOLS_H
#define ELASTICITYTENSORTOOLS_H

template <typename>
class RankFourTensorTempl;
typedef RankFourTensorTempl<Real> RankFourTensor;
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
Real getIsotropicShearModulus(const RankFourTensor & elasticity_tensor);

/**
 * Get the bulk modulus for an isotropic elasticity tensor
 * param elasticity_tensor the tensor (must be isotropic, but not checked for efficiency)
 */
Real getIsotropicBulkModulus(const RankFourTensor & elasticity_tensor);

/**
 * Get the Young's modulus for an isotropic elasticity tensor
 * param elasticity_tensor the tensor (must be isotropic, but not checked for efficiency)
 */
Real getIsotropicYoungsModulus(const RankFourTensor & elasticity_tensor);

/**
 * Get the Poisson's modulus for an isotropic elasticity tensor
 * param elasticity_tensor the tensor (must be isotropic, but not checked for efficiency)
 */
Real getIsotropicPoissonsRatio(const RankFourTensor & elasticity_tensor);
}

#endif // ELASTICITYTENSORTOOLS_H
