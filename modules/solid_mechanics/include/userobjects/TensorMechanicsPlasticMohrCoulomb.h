//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TensorMechanicsPlasticModel.h"
#include "TensorMechanicsHardeningModel.h"

/**
 * Mohr-Coulomb plasticity, nonassociative with hardening/softening.
 *
 * For 'hyperbolic' smoothing, the smoothing of the tip of the yield-surface cone is described in
 * Zienkiewicz and Prande "Some useful forms of isotropic yield surfaces for soil and rock
 * mechanics" (1977) In G Gudehus (editor) "Finite Elements in Geomechanics" Wile, Chichester, pp
 * 179-190.
 * For 'cap' smoothing, additional smoothing is performed.
 * The smoothing of the edges of the cone is described in
 * AJ Abbo, AV Lyamin, SW Sloan, JP Hambleton "A C2 continuous approximation to the Mohr-Coulomb
 * yield surface" International Journal of Solids and Structures 48 (2011) 3001-3010
 *
 */
class TensorMechanicsPlasticMohrCoulomb : public TensorMechanicsPlasticModel
{
public:
  static InputParameters validParams();

  TensorMechanicsPlasticMohrCoulomb(const InputParameters & parameters);

  virtual std::string modelName() const override;

protected:
  Real yieldFunction(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, Real intnl) const override;

  Real dyieldFunction_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor flowPotential(const RankTwoTensor & stress, Real intnl) const override;

  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  /// Hardening model for cohesion
  const TensorMechanicsHardeningModel & _cohesion;

  /// Hardening model for phi
  const TensorMechanicsHardeningModel & _phi;

  /// Hardening model for psi
  const TensorMechanicsHardeningModel & _psi;

  /**
   * The yield function is modified to
   * f = s_m*sinphi + sqrt(a + s_bar^2 K^2) - C*cosphi
   * where "a" depends on the tip_scheme.  Currently _tip_scheme is
   * 'hyperbolic', where a = _small_smoother2
   * 'cap' where a = _small_smoother2 + (p(stress_mean - _cap_start))^2
   *       with the function p(x)=x(1-exp(-_cap_rate*x)) for x>0, and p=0 otherwise
   */
  MooseEnum _tip_scheme;

  /// Square of tip smoothing parameter to smooth the cone at mean_stress = T
  Real _small_smoother2;

  /// smoothing parameter dictating when the 'cap' will start - see doco for _tip_scheme
  Real _cap_start;

  /// dictates how quickly the 'cap' degenerates to a hemisphere - see doco for _tip_scheme
  Real _cap_rate;

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

  /// returns the 'a' parameter - see doco for _tip_scheme
  virtual Real smooth(const RankTwoTensor & stress) const;

  /// returns the da/dstress_mean - see doco for _tip_scheme
  virtual Real dsmooth(const RankTwoTensor & stress) const;

  /// returns the d^2a/dstress_mean^2 - see doco for _tip_scheme
  virtual Real d2smooth(const RankTwoTensor & stress) const;

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
   * @param sin_angle sin(friction_angle) (for yield function), or sin(dilation_angle) (for
   * potential function)
   * @param[out] aaa Abbo's A
   * @param[out] bbb Abbo's B
   * @param[out] ccc Abbo's C
   */
  void abbo(const Real sin3lode, const Real sin_angle, Real & aaa, Real & bbb, Real & ccc) const;

  /**
   * Computes derivatives of Abbo et al's A, B and C parameters wrt sin_angle
   * @param sin3lode sin(3*(lode angle))
   * @param sin_angle sin(friction_angle) (for yield function), or sin(dilation_angle) (for
   * potential function)
   * @param[out] daaa d(Abbo's A)/d(sin_angle)
   * @param[out] dbbb d(Abbo's B)/d(sin_angle)
   * @param[out] dccc d(Abbo's C)/d(sin_angle)
   */
  void
  dabbo(const Real sin3lode, const Real sin_angle, Real & daaa, Real & dbbb, Real & dccc) const;

  /**
   * d(yieldFunction)/d(stress), but with the ability to put friction or dilation angle into the
   * result
   * @param stress the stress at which to calculate
   * @param sin_angle either sin(friction angle) or sin(dilation angle)
   */
  RankTwoTensor df_dsig(const RankTwoTensor & stress, const Real sin_angle) const;
};
