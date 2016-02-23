/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef ELASTICITYTENSORR4_H
#define ELASTICITYTENSORR4_H

#include "RankFourTensor.h"
#include "DerivativeMaterialInterface.h"

class ElasticityTensorR4;

/**
 * Helper function template specialization to set an object to zero.
 * Needed by DerivativeMaterialInterface
 */
template<>
void mooseSetToZero<ElasticityTensorR4>(ElasticityTensorR4 & v);

/**
 * ElasticityTensorR4 inherits from RankFourTensor.  As such it contains data and
 * methods useful for a general elasticity tensor, C_ijkl, with 81 components (in 3D).
 * RankFourTensor has various filling methods that allow users to specify
 * particular symmetries, hence reducing the number of independent components to < 81.
 *
 * This class contains methods to calculate Jacobians for kernels.
 *
 * Original class authors: A. M. Jokisaari, O. Heinonen, M.R. Tonks
 */
class ElasticityTensorR4 : public RankFourTensor
{
public:
  virtual ~ElasticityTensorR4() {}

  /// Default constructor; fills to zero
  ElasticityTensorR4();

  /// Copy constructor
  ElasticityTensorR4(const ElasticityTensorR4 &a);

  /// Copy constructor for RankFourTensor
  ElasticityTensorR4(const RankFourTensor &a);

  // Fill from vector
  ElasticityTensorR4(const std::vector<Real> & input, FillMethod fill_method) : RankFourTensor(input, fill_method) {}

  /**
   * This is used for the standard kernel stress_ij*d(test)/dx_j, when varied wrt u_k
   * Jacobian entry: d(stress_ij*d(test)/dx_j)/du_k = d(C_ijmn*du_m/dx_n*dtest/dx_j)/du_k
   */
  virtual Real elasticJacobian( const unsigned int i, const unsigned int k, const RealGradient & grad_test, const RealGradient & grad_phi) const;

  /**
   * This is used for the standard kernel stress_ij*d(test)/dx_j, when varied wrt w_k (the cosserat rotation)
   * Jacobian entry: d(stress_ij*d(test)/dx_j)/dw_k = d(C_ijmn*eps_mnp*w_p*dtest/dx_j)/dw_k
   */
  virtual Real elasticJacobianwc(const unsigned int i, const unsigned int k, const RealGradient & grad_test, Real phi) const;

  /**
   * This is used for the moment-balancing kernel eps_ijk*stress_jk*test, when varied wrt u_k
   * Jacobian entry: d(eps_ijm*stress_jm*test)/du_k = d(eps_ijm*C_jmln*du_l/dx_n*test)/du_k
   */
  virtual Real momentJacobian(const unsigned int i, const unsigned int k, Real test, const RealGradient & grad_phi) const;

  /**
   * This is used for the moment-balancing kernel eps_ijk*stress_jk*test, when varied wrt w_k (the cosserat rotation)
   * Jacobian entry: d(eps_ijm*stress_jm*test)/dw_k = d(eps_ijm*C_jmln*eps_lnp*w_p*test)/dw_k
   */
  virtual Real momentJacobianwc(const unsigned int i, const unsigned int k, Real test, Real phi) const;
};

#endif //ELASTICITYTENSORR4_H
