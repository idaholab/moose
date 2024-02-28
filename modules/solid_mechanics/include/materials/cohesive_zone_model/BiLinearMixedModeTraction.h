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

  virtual void computeInterfaceTractionAndDerivatives() override;

  /// The traction-separation law
  virtual RealVectorValue computeTraction();

  /// Compute the total traction derivatives w.r.t. the interface displacement jump
  virtual RankTwoTensor computeTractionDerivatives();

  // The damage evolution law
  virtual void computeDamage();

  /// Penalty elastic stiffness
  const Real _K;

  ///@{
  /// damage variable
  MaterialProperty<Real> & _d;
  const MaterialProperty<Real> & _d_old;
  ///@}

  /// old interface displacement jump value
  const MaterialProperty<RealVectorValue> & _interface_displacement_jump_old;

  // @{
  // The parameters in the damage evolution law
  MaterialProperty<Real> & _delta_init;
  MaterialProperty<Real> & _delta_final;
  MaterialProperty<Real> & _delta_m;
  // @}

  /// Mode I critical fracture toughness
  const MaterialProperty<Real> & _GI_c;

  /// Mode II critical fracture toughness
  const MaterialProperty<Real> & _GII_c;

  /// The normal strength
  const MaterialProperty<Real> & _N;

  /// The shear strength
  const MaterialProperty<Real> & _S;

  /// The B-K power law parameter
  const Real _eta;

  /// The mode mixity ratio
  MaterialProperty<Real> & _beta;

  /// The viscosity
  const Real _viscosity;

  /// mixed mode propagation criterion
  enum class MixedModeCriterion
  {
    POWER_LAW,
    BK
  } _criterion;

private:
  // Compute mode mixity ratio
  void computeModeMixity();

  // Compute relative displacement jump at damage initiation
  void computeCriticalDisplacementJump();

  // Compute relative displacement jump at full degradation
  void computeFinalDisplacementJump();

  // Compute mixed mode relative displacement
  void computeEffectiveDisplacementJump();

  // @{
  // Parameters to improve numerical convergence
  const bool _lag_mode_mixity;
  const bool _lag_disp_jump;
  const Real _alpha;
  // @}

  // @{
  // Intermediate derivatives used in the chain rule for computing the derivative of damage w.r.t.
  // displacement jumps
  RealVectorValue _dbeta_ddelta;
  RealVectorValue _ddelta_init_ddelta;
  RealVectorValue _ddelta_final_ddelta;
  RealVectorValue _ddelta_m_ddelta;
  RealVectorValue _dd_ddelta;
  // @}
};
