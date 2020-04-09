//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TensorMechanicsPlasticJ2.h"
#include "RankFourTensor.h"

/**
 * IsotropicSD plasticity model from Yoon (2013)
 * the name of the paper is "Asymmetric yield function based on the
 * stress invariants for pressure sensitive metals" published
 * 4th December 2013.
 * This model accounts for sensitivity in pressure and for the
 * strength differential effect
 * Yield_function = \f$ a[b*I_{1} + (J2^{3/2} - c*J3)^{1/3}]\f$ - yield_strength
 * The last three functions are the main functions that call all other
 * functions in this module for the Newton-Raphson method.
 */
class TensorMechanicsPlasticIsotropicSD : public TensorMechanicsPlasticJ2
{
public:
  static InputParameters validParams();

  TensorMechanicsPlasticIsotropicSD(const InputParameters & parameters);

protected:
  /// A constant to model the influence of pressure
  const Real _b;

  /// A constant to model the influence of strength differential effect
  Real _c;

  /// Flag for flow-rule, true if not specified
  const bool _associative;

  /// Comes from transforming the stress tensor to the deviatoric stress tensor
  RankFourTensor _h;

  /// A constant used in the constructor that depends on _b and _c
  Real _a;

  /// derivative of phi with respect to J2, phi is b*I1 + (J2^{3/2} - c*J3)^{1/3}
  Real dphi_dj2(const Real j2, const Real j3) const;

  /// derivative of phi with respect to J3
  Real dphi_dj3(const Real j2, const Real j3) const;

  /// derivative of dphi_dJ2 with respect to J2
  Real dfj2_dj2(const Real j2, const Real j3) const;

  /// derivative of dphi_dJ2 with respect to J3
  Real dfj2_dj3(const Real j2, const Real j3) const;

  /// derivative of dphi_dJ3 with respect to J2
  Real dfj3_dj2(const Real j2, const Real j3) const;

  /// derivative of dphi_dJ3 with respect to J3
  Real dfj3_dj3(const Real j2, const Real j3) const;

  /// derivative of the trace with respect to sigma rank two tensor
  RankTwoTensor dI_sigma() const;

  /// derivative of the second invariant with respect to the stress deviatoric tensor
  RankTwoTensor dj2_dSkl(const RankTwoTensor & stress) const;

  /// Yield_function = a[b*I1 + (J2^{3/2} - c*J3)^{1/3}] - yield_strength
  Real yieldFunction(const RankTwoTensor & stress, Real intnl) const override;

  /// Tensor derivative of the yield_function with respect to the stress tensor
  RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, Real intnl) const override;

  /// Tensor derivative of the tensor derivative of the yield_function with respect to the stress tensor
  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, Real intnl) const override;

  /// Receives the flag for associative or non-associative and calculates the flow potential accordingly
  RankTwoTensor flowPotential(const RankTwoTensor & stress, Real intnl) const override;
};
