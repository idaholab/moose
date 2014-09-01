#ifndef TENSORMECHANICSPLASTICMOHRCOULOMB_H
#define TENSORMECHANICSPLASTICMOHRCOULOMB_H

#include "TensorMechanicsPlasticModel.h"


class TensorMechanicsPlasticMohrCoulomb;


template<>
InputParameters validParams<TensorMechanicsPlasticMohrCoulomb>();

/**
 * Mohr-Coulomb plasticity, nonassociative with hardening/softening
 *
 * The smoothing of the tip of the yield-surface cone is described in
 * Zienkiewicz and Prande "Some useful forms of isotropic yield surfaces for soil and rock mechanics" (1977) In G Gudehus (editor) "Finite Elements in Geomechanics" Wile, Chichester, pp 179-190.
 * The smoothing of the edges of the cone is described in
 * AJ Abbo, AV Lyamin, SW Sloan, JP Hambleton "A C2 continuous approximation to the Mohr-Coulomb yield surface" International Journal of Solids and Structures 48 (2011) 3001-3010
 */
class TensorMechanicsPlasticMohrCoulomb : public TensorMechanicsPlasticModel
{
 public:
  TensorMechanicsPlasticMohrCoulomb(const std::string & name, InputParameters parameters);

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

  /// The cohesion at zero hardening
  Real _cohesion;

  /// friction angle at zero hardening
  Real _phi;

  /// dilation angle at zero hardening
  Real _psi;

  /// The cohesion_residual
  Real _cohesion_residual;

  /// friction angle_residual
  Real _phi_residual;

  /// dilation angle_residual
  Real _psi_residual;

  /// Logarithmic rate of change of cohesion to _cohesion_residual
  Real _cohesion_rate;

  /// Logarithmic rate of change of _phi to _phi_residual
  Real _phi_rate;

  /// Logarithmic rate of change of _psi to _psi_residual
  Real _psi_rate;

  /// Square of tip smoothing parameter to smooth the cone at mean_stress = T
  Real _small_smoother2;

  /// edge smoothing parameter, in radians
  Real _tt;

  /// cos(_tt)
  Real _costt;

  /// sin(_tt)
  Real _sintt;

  /// cos(3*_tt)
  Real _cos3tt;

  /// sin(3*_tt) - useful for making comparisons with Lode angle
  Real _sin3tt;

  /// cos(6*_tt)
  Real _cos6tt;

  /// sin(6*_tt)
  Real _sin6tt;

  /// if secondInvariant < _lode_cutoff then set Lode angle to zero.  This is to guard against precision-loss
  Real _lode_cutoff;

  /// cohesion as a function of internal parameter
  virtual Real cohesion(const Real internal_param) const;

  /// d(cohesion)/d(internal_param);
  virtual Real dcohesion(const Real internal_param) const;

  /// friction angle as a function of internal parameter
  virtual Real phi(const Real internal_param) const;

  /// d(phi)/d(internal_param);
  virtual Real dphi(const Real internal_param) const;

  /// dilation angle as a function of internal parameter
  virtual Real psi(const Real internal_param) const;

  /// d(psi)/d(internal_param);
  virtual Real dpsi(const Real internal_param) const;


 private:

  /**
   * Computes Abbo et al's A, B and C parameters
   * @param sin3lode sin(3*(lode angle))
   * @param sin_angle sin(friction_angle) (for yield function), or sin(dilation_angle) (for potential function)
   * @param aaa (output) Abbo's A
   * @param bbb (output) Abbo's B
   * @param ccc (output) Abbo's C
   */
  void abbo(const Real sin3lode, const Real sin_angle, Real & aaa, Real & bbb, Real & ccc) const;

  /**
   * Computes derivatives of Abbo et al's A, B and C parameters wrt sin_angle
   * @param sin3lode sin(3*(lode angle))
   * @param sin_angle sin(friction_angle) (for yield function), or sin(dilation_angle) (for potential function)
   * @param daaa (output) d(Abbo's A)/d(sin_angle)
   * @param dbbb (output) d(Abbo's B)/d(sin_angle)
   * @param dccc (output) d(Abbo's C)/d(sin_angle)
   */
  void dabbo(const Real sin3lode, const Real sin_angle, Real & daaa, Real & dbbb, Real & dccc) const;

  /**
   * d(yieldFunction)/d(stress), but with the ability to put friction or dilation angle into the result
   * @param stress the stress at which to calculate
   * @param sin_angle either sin(friction angle) or sin(dilation angle)
   */
  RankTwoTensor df_dsig(const RankTwoTensor & stress, const Real sin_angle) const;


};

#endif // TENSORMECHANICSPLASTICMOHRCOULOMB_H
