//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FiniteStrainHyperElasticViscoPlastic.h"

/**
 * This class solves visco plastic model based on isotropically damaged stress
 * The damage parameter is obtained from phase field fracture kernel
 * Computes undamaged elastic strain energy and associated tensors used in phase field fracture
 * kernel
 */
class HyperElasticPhaseFieldIsoDamage : public FiniteStrainHyperElasticViscoPlastic
{
public:
  static InputParameters validParams();

  HyperElasticPhaseFieldIsoDamage(const InputParameters & parameters);

protected:
  /// This function computes PK2 stress
  virtual void computePK2StressAndDerivative();
  /**
   * This function computes PK2 stress modified to account for damage
   * Computes numerical stiffness if flag is true
   * Computes undamaged elastic strain energy and associated tensors
   */
  virtual void computeDamageStress();
  /// This function computes numerical stiffness
  virtual void computeNumStiffness();
  /// This function computes tensors used to construct diagonal and off-diagonal Jacobian
  virtual void computeQpJacobian();

  /// Flag to compute numerical stiffness
  bool _num_stiffness;
  /// Small stiffness of completely damaged material point
  Real _kdamage;
  /// Use current value of history variable
  bool _use_current_hist;
  /// Material property defining crack width, declared elsewhere
  const MaterialProperty<Real> & _l;
  /// Material property defining gc parameter, declared elsewhere
  const MaterialProperty<Real> & _gc;
  /// Used in numerical stiffness calculation to check near zero values
  Real _zero_tol;
  /// Perturbation value for near zero or zero strain components
  Real _zero_pert;
  /// Perturbation value for strain components
  Real _pert_val;
  /// Compupled damage variable
  const VariableValue & _c;
  /// Flag to save couple material properties
  bool _save_state;

  MaterialProperty<RankTwoTensor> & _dstress_dc;

  RankTwoTensor _pk2_tmp, _dG0_dee, _dpk2_dc;
  RankFourTensor _dpk2_dee;
  std::vector<RankTwoTensor> _etens;

  /// Elastic energy and derivatives, declared in this material
  MaterialProperty<Real> & _F;
  MaterialProperty<Real> & _dFdc;
  MaterialProperty<Real> & _d2Fdc2;
  MaterialProperty<RankTwoTensor> & _d2Fdcdstrain;

  /// History variable that prevents crack healing, declared in this material
  MaterialProperty<Real> & _hist;

  /// Old value of history variable
  const MaterialProperty<Real> & _hist_old;
};
