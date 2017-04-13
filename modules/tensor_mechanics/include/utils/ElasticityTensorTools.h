/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ELASTICITYTENSORTOOLS_H
#define ELASTICITYTENSORTOOLS_H

class RankFourTensor;

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
}

#endif // ELASTICITYTENSORTOOLS_H
