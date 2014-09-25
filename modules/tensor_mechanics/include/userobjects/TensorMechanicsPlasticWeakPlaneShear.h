#ifndef TENSORMECHANICSPLASTICWEAKPLANESHEAR_H
#define TENSORMECHANICSPLASTICWEAKPLANESHEAR_H

#include "TensorMechanicsPlasticModel.h"


class TensorMechanicsPlasticWeakPlaneShear;


template<>
InputParameters validParams<TensorMechanicsPlasticWeakPlaneShear>();

/**
 * Rate-independent associative weak-plane tensile failure
 * with hardening/softening
 */
class TensorMechanicsPlasticWeakPlaneShear : public TensorMechanicsPlasticModel
{
 public:
  TensorMechanicsPlasticWeakPlaneShear(const std::string & name, InputParameters parameters);

  /**
   * The yield function
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return the yield function
   */
  Real yieldFunction(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of yield function with respect to stress
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return df_dstress(i, j) = dyieldFunction/dstress(i, j)
   */
  RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of yield function with respect to the internal parameter
   * @param stress the stress at which to calculate the yield function
   * @param intnl internal parameter
   * @return the derivative
   */
  Real dyieldFunction_dintnl(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The flow potential
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return the flow potential
   */
  RankTwoTensor flowPotential(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of the flow potential with respect to stress
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return dr_dstress(i, j, k, l) = dr(i, j)/dstress(k, l)
   */
  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, const Real & intnl) const;

  /**
   * The derivative of the flow potential with respect to the internal parameter
   * @param stress the stress at which to calculate the flow potential
   * @param intnl internal parameter
   * @return dr_dintnl(i, j) = dr(i, j)/dintnl
   */
  RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, const Real & intnl) const;

 protected:

  /// The cohesion
  Real _cohesion;

  /// tan(friction angle)
  Real _tan_phi;

  /// tan(dilation angle)
  Real _tan_psi;

  /// The cohesion_residual
  Real _cohesion_residual;

  /// tan(friction angle_residual)
  Real _tan_phi_residual;

  /// tan(dilation angle_residual)
  Real _tan_psi_residual;

  /// Logarithmic rate of change of cohesion to _cohesion_residual
  Real _cohesion_rate;

  /// Logarithmic rate of change of tan_phi to tan_phi_residual
  Real _tan_phi_rate;

  /// Logarithmic rate of change of tan_psi to tan_psi_residual
  Real _tan_psi_rate;

  /**
   * The yield function needs to be smooth around shear-stress=0,
   * so it is modified to be
   * f = sqrt(s_xz^2 + s_yz^2 + (_small_smoother*_cohesion)^2) + s_zz*_tan_phi - _cohesion
   */
  Real _small_smoother;

  /// Function that's used in dyieldFunction_dstress and flowPotential
  RankTwoTensor df_dsig(const RankTwoTensor & stress, const Real & _tan_phi_or_psi) const;

  /// cohesion as a function of internal parameter
  virtual Real cohesion(const Real internal_param) const;

  /// d(cohesion)/d(internal_param)
  virtual Real dcohesion(const Real internal_param) const;

  /// tan_phi as a function of internal parameter
  virtual Real tan_phi(const Real internal_param) const;

  /// d(tan_phi)/d(internal_param);
  virtual Real dtan_phi(const Real internal_param) const;

  /// tan_psi as a function of internal parameter
  virtual Real tan_psi(const Real internal_param) const;

  /// d(tan_psi)/d(internal_param);
  virtual Real dtan_psi(const Real internal_param) const;

};

#endif // TENSORMECHANICSPLASTICWEAKPLANESHEAR_H
