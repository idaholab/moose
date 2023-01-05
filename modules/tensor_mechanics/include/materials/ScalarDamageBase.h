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

/**
 * Base class for scalar damage models.
 */
template <bool is_ad>
class ScalarDamageBaseTempl : public DamageBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  ScalarDamageBaseTempl(const InputParameters & parameters);

  virtual void initQpStatefulProperties() override;

  virtual void updateDamage() override;

  virtual void updateStressForDamage(GenericRankTwoTensor<is_ad> & stress_new) override;

  virtual void updateJacobianMultForDamage(RankFourTensor & jacobian_mult) override;

  virtual void computeUndamagedOldStress(RankTwoTensor & stress_old) override;

  virtual Real computeTimeStepLimit() override;

  /**
   * Get the value of the damage index for the current quadrature point.
   */
  const GenericReal<is_ad> & getQpDamageIndex(unsigned int qp);

  /**
   * Get the name of the material property containing the damage index
   */
  const std::string getDamageIndexName() const { return _damage_index_name; }

protected:
  /// Name of the material property where the damage index is stored
  const MaterialPropertyName _damage_index_name;

  /// Update the damage index at the current qpoint
  virtual void updateQpDamageIndex() = 0;

  ///@{ Material property that provides the damage index
  GenericMaterialProperty<Real, is_ad> & _damage_index;
  const MaterialProperty<Real> & _damage_index_old;
  const MaterialProperty<Real> & _damage_index_older;
  ///@}

  /// If true, use the damage index from the old state (rather than the current state)
  const bool _use_old_damage;

  /// Residual fraction of stiffness used for material that is fully damaged
  const Real & _residual_stiffness_fraction;

  /// Maximum damage increment allowed for the time step
  const Real & _maximum_damage_increment;

  /// Maximum allowed value for the damage index
  const Real & _maximum_damage;

  using DamageBaseTempl<is_ad>::_qp;
  using DamageBaseTempl<is_ad>::_base_name;
  using DamageBaseTempl<is_ad>::setQp;
  using DamageBaseTempl<is_ad>::_dt;
};

typedef ScalarDamageBaseTempl<false> ScalarDamageBase;
typedef ScalarDamageBaseTempl<true> ADScalarDamageBase;
