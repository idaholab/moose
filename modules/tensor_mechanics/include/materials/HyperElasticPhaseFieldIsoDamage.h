/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#ifndef HYPERELASTICPHASEFIELDISODAMAGE_H
#define HYPERELASTICPHASEFIELDISODAMAGE_H

#include "FiniteStrainHyperElasticViscoPlastic.h"

class HyperElasticPhaseFieldIsoDamage;

template <>
InputParameters validParams<HyperElasticPhaseFieldIsoDamage>();

/**
 * This class solves visco plastic model based on isotropically damaged stress
 * The damage parameter is obtained from phase field fracture kernel
 * Computes undamaged elastic strain energy and associated tensors used in phase field fracture
 * kernel
 */
class HyperElasticPhaseFieldIsoDamage : public FiniteStrainHyperElasticViscoPlastic
{
public:
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

  MaterialProperty<Real> & _G0;
  MaterialProperty<RankTwoTensor> & _dG0_dstrain;
  MaterialProperty<RankTwoTensor> & _dstress_dc;

  RankTwoTensor _pk2_tmp, _dG0_dee, _dpk2_dc;
  RankFourTensor _dpk2_dee;
  std::vector<RankTwoTensor> _etens;
};

#endif
