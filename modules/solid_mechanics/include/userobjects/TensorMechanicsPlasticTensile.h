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
 * FiniteStrainTensile implements rate-independent associative tensile failure
 * with hardening/softening in the finite-strain framework.
 * For 'hyperbolic' smoothing, the smoothing of the tip of the yield-surface cone is described in
 * Zienkiewicz and Prande "Some useful forms of isotropic yield surfaces for soil and rock
 * mechanics" (1977) In G Gudehus (editor) "Finite Elements in Geomechanics" Wile, Chichester, pp
 * 179-190.
 * For 'cap' smoothing, additional smoothing is performed.
 * The smoothing of the edges of the cone is described in
 * AJ Abbo, AV Lyamin, SW Sloan, JP Hambleton "A C2 continuous approximation to the Mohr-Coulomb
 * yield surface" International Journal of Solids and Structures 48 (2011) 3001-3010
 */
class TensorMechanicsPlasticTensile : public TensorMechanicsPlasticModel
{
public:
  static InputParameters validParams();

  TensorMechanicsPlasticTensile(const InputParameters & parameters);

  virtual std::string modelName() const override;

protected:
  Real yieldFunction(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, Real intnl) const override;

  Real dyieldFunction_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor flowPotential(const RankTwoTensor & stress, Real intnl) const override;

  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, Real intnl) const override;

  RankTwoTensor dflowPotential_dintnl(const RankTwoTensor & stress, Real intnl) const override;

  const TensorMechanicsHardeningModel & _strength;

  /**
   * The yield function is modified to
   * f = s_m + sqrt(a + s_bar^2 K^2) - tensile_strength
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

  /// sin(3*_tt) - useful for making comparisons with Lode angle
  Real _sin3tt;

  /// if secondInvariant < _lode_cutoff then set Lode angle to zero.  This is to guard against precision-loss
  Real _lode_cutoff;

  /// Abbo et al's C parameter
  Real _ccc;

  /// Abbo et al's B parameter
  Real _bbb;

  /// Abbo et al's A parameter
  Real _aaa;

  /// returns the 'a' parameter - see doco for _tip_scheme
  virtual Real smooth(const RankTwoTensor & stress) const;

  /// returns the da/dstress_mean - see doco for _tip_scheme
  virtual Real dsmooth(const RankTwoTensor & stress) const;

  /// returns the d^2a/dstress_mean^2 - see doco for _tip_scheme
  virtual Real d2smooth(const RankTwoTensor & stress) const;

  /// tensile strength as a function of residual value, rate, and internal_param
  virtual Real tensile_strength(const Real internal_param) const;

  /// d(tensile strength)/d(internal_param) as a function of residual value, rate, and internal_param
  virtual Real dtensile_strength(const Real internal_param) const;
};
