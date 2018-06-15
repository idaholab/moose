//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SCALARMATERIALDAMAGE_H
#define SCALARMATERIALDAMAGE_H

#include "DamageBase.h"

// Forward declaration
class ScalarMaterialDamage;

template <>
InputParameters validParams<ScalarMaterialDamage>();

/**
 * Scalar damage model for which the damage is prescribed by another material
 */
class ScalarMaterialDamage : public DamageBase
{
public:
  ScalarMaterialDamage(const InputParameters & parameters);

  virtual void updateDamage() override;

  virtual void updateStressForDamage(RankTwoTensor & stress_new) override;

  virtual void updateJacobianMultForDamage(RankFourTensor & jacobian_mult) override;

  virtual Real computeTimeStepLimit() override;

protected:
  /// If true, use the damage index from the old state (rather than the current state)
  const bool _use_old_damage;

  ///@{ Material property that provides the damage index
  const MaterialProperty<Real> & _damage_index;
  const MaterialProperty<Real> & _damage_index_old;
  ///@}

  /// Residual fraction of stiffness used for material that is fully damaged
  const Real & _residual_stiffness_fraction;

  /// Maximum damage increment allowed for the time step
  const Real & _maximum_damage_increment;
};

#endif // SCALARMATERIALDAMAGE_H
