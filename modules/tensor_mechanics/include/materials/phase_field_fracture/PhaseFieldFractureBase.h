//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DamageBase.h"
#include "DerivativeMaterialInterface.h"

/**
 * Base class for phase-field fracture damage models.
 */
class PhaseFieldFractureBase : public DerivativeMaterialInterface<DamageBase>
{
public:
  static InputParameters validParams();

  PhaseFieldFractureBase(const InputParameters & parameters);

  virtual void initQpStatefulProperties() override;

  void updateStressForDamage(RankTwoTensor & stress_new) override final;

  virtual void computeUndamagedOldStress(RankTwoTensor & stress_old) override;

  virtual Real computeTimeStepLimit() override;

  virtual void finiteStrainRotation(const RankTwoTensor & rotation_increment) override;

protected:
  /// Compute the damaged stress after computing the undamaged stress
  virtual void computeDamagedStress(RankTwoTensor & stress_new) = 0;

  /// Compute the elastic energy based on damaged stress/strains
  virtual void computeElasticEnergy() = 0;

  // {@ Macaulay bracket operator
  virtual Real Macaulay(const Real x, const bool deriv = false) const;
  virtual std::vector<Real> Macaulay(const std::vector<Real> & x, const bool deriv = false) const;
  // @}

  /// The undamaged elasticity tensor
  const MaterialProperty<RankFourTensor> & _elasticity_tensor;

  //@{ The phase-field (damage) variable
  const VariableValue & _c;
  const VariableValue & _c_old;
  const VariableName _c_name;
  //@}

  //@{ The elastic energy and its derivatives w/r/t damage
  const MaterialPropertyName _E_name;
  MaterialProperty<Real> & _E;
  MaterialProperty<Real> & _E_active;
  const MaterialProperty<Real> & _E_active_old;
  MaterialProperty<Real> & _dE_dc;
  MaterialProperty<Real> & _d2E_dc2;
  MaterialProperty<RankTwoTensor> & _d2E_dcdstrain;
  //@}

  //@{ The degradation function and its derivatives w/r/t damage
  const MaterialPropertyName _g_name;
  const MaterialProperty<Real> & _g;
  const MaterialProperty<Real> & _dg_dc;
  const MaterialProperty<Real> & _d2g_dc2;
  //@}

  //@{ Strains and stresses
  const MaterialProperty<RankTwoTensor> & _elastic_strain;
  MaterialProperty<RankTwoTensor> & _undamaged_stress;
  const MaterialProperty<RankTwoTensor> & _undamaged_stress_old;
  MaterialProperty<RankTwoTensor> & _stress_pos;
  MaterialProperty<RankTwoTensor> & _dstress_dc;
  //@}

  /// Whether to use the old elastic energy
  const bool _use_old_elastic_energy;

  /// Maximum damage increment allowed for the time step
  const Real _maximum_damage_increment;

  /// Whether to use the hybrid formulation
  const bool _hybrid;
};
