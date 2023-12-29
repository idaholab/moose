//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "TensorMechanicsPlasticIsotropicSD.h"

/**
 * Orthotropic plasticity model from Yoon (2013)
 * the name of the paper is "Asymmetric yield function based on the
 * stress invariants for pressure sensitive metals" published
 * 4th December 2013.
 * This model accounts for sensitivity in pressure, the
 * strength differential effect and orthotropic behavior
 * Yield_function = \f$ b*I_{1} + (J'2^{3/2} - c*J''3)^{1/3}\f$ - yield_strength
 * The last three functions are the main functions that call all other
 * functions in this module for the Newton-Raphson method.
 */
class TensorMechanicsPlasticOrthotropic : public TensorMechanicsPlasticIsotropicSD
{
public:
  static InputParameters validParams();

  TensorMechanicsPlasticOrthotropic(const InputParameters & parameters);

protected:
  /// The six coefficients of L prime
  const std::vector<Real> _c1;

  /// The six coefficients of L prime prime
  const std::vector<Real> _c2;

  /// Transformation tensor from the stress tensor to the deviatoric stress tensor for J2
  RankFourTensor _l1;

  /// Transformation tensor from the stress tensor to the deviatoric stress tensor for J3
  RankFourTensor _l2;

  /// Yield_function = a[b*I1 + (J2^{3/2} - c*J3)^{1/3}] - yield_strength
  Real yieldFunction(const RankTwoTensor & stress, Real intnl) const override;

  /// Tensor derivative of the yield_function with respect to the stress tensor
  RankTwoTensor dyieldFunction_dstress(const RankTwoTensor & stress, Real intnl) const override;

  /// Tensor derivative of the tensor derivative of the yield_function with respect to the stress tensor
  RankFourTensor dflowPotential_dstress(const RankTwoTensor & stress, Real intnl) const override;

  /// Receives the flag for associative or non-associative and calculates the flow potential accordingly
  RankTwoTensor flowPotential(const RankTwoTensor & stress, Real intnl) const override;
};
