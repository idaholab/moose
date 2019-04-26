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

// Forward declaration
class ScalarDamageBase;

template <>
InputParameters validParams<ScalarDamageBase>();

/**
 * Base class for scalar damage models.
 */
class ScalarDamageBase : public DamageBase
{
public:
  ScalarDamageBase(const InputParameters & parameters);

  virtual void initQpStatefulProperties() override;

  virtual void updateDamage() override;

  virtual void updateStressForDamage(RankTwoTensor & stress_new) override;

  virtual void updateJacobianMultForDamage(RankFourTensor & jacobian_mult) override;

  virtual Real computeTimeStepLimit() override;

  const Real & getQpDamageIndex(unsigned int qp);

  const std::string getDamageIndexName() const { return _damage_index_name; }

protected:
  /// Name of the material property where the damage index is stored
  const std::string _damage_index_name;

  /// Update the damage index at the current qpoint
  virtual void updateQpDamageIndex() = 0;

  ///@{ Material property that provides the damage index
  MaterialProperty<Real> & _damage_index;
  const MaterialProperty<Real> & _damage_index_old;
  ///@}

  /// If true, use the damage index from the old state (rather than the current state)
  const bool _use_old_damage;

  /// Residual fraction of stiffness used for material that is fully damaged
  const Real & _residual_stiffness_fraction;

  /// Maximum damage increment allowed for the time step
  const Real & _maximum_damage_increment;
};

