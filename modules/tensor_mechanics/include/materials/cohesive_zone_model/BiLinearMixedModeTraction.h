//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "CZMComputeLocalTractionTotalBase.h"

/**
 * Implementation of the mixed mode bilinear traction separation law
 * described in Mixed-Mode Decohesion Finite Elements for the Simulation of Delamination in
 *Composite Materials, Pedro P. Camanho and Carlos G. Davila, NASA/TM-2002-211737
 **/
class BiLinearMixedModeTraction : public CZMComputeLocalTractionTotalBase
{
public:
  static InputParameters validParams();
  BiLinearMixedModeTraction(const InputParameters & parameters);

protected:
  virtual void initQpStatefulProperties() override;

  void computeInterfaceTractionAndDerivatives() override;

  /// method computing the total traction
  RealVectorValue computeTraction();

  /// method computing the total traction derivatives w.r.t. the interface displacement jump
  RankTwoTensor computeTractionDerivatives();

  /// scale factor to vary normal_strength
  const VariableValue * const _scale_factor;

  /// penalty elastic stiffness
  const Real _stiffness;

  ///@{
  /// damage variable
  MaterialProperty<Real> & _d;
  const MaterialProperty<Real> & _d_old;
  ///@}

  /// old interface displacement jump value
  const MaterialProperty<RealVectorValue> & _interface_displacement_jump_old;

  /// viscous regularization
  const Real _viscosity;

  ///@{
  /// critical Mode I and II fracture toughness
  const Real _GI_C;
  const Real _GII_C;
  ///@}

  ///@{
  /// onset normal seperation and shear seperation
  const Real _delta_normal0;
  const Real _delta_shear0;
  ///@}

  /// effective displacement at damage initiation
  MaterialProperty<Real> & _eff_disp_damage_init;

  /// effective displacement at full degradation
  MaterialProperty<Real> & _eff_disp_full_degradation;

  /// The B-K power law parameter
  const Real _eta;

  ///@{
  /// maximum mixed mode relative displacement
  MaterialProperty<Real> & _maximum_mixed_mode_relative_displacement;
  const MaterialProperty<Real> & _maximum_mixed_mode_relative_displacement_old;
  ///@}

  /// square of mode_mixity_ratio
  Real _beta_sq;

  /// local derivative of traction w.r.t. interface jump
  RankTwoTensor _D;

  /// local derivative of stiffness w.r.t. interface jump
  RankTwoTensor _dDdu;

  /// mode_mixity_ratio
  MaterialProperty<Real> & _beta;
};
