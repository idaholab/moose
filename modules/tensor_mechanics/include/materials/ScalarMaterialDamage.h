//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ScalarDamageBase.h"

// Forward declaration

/**
 * Scalar damage model for which the damage is prescribed by another material
 */
template <bool is_ad>
class ScalarMaterialDamageTempl : public ScalarDamageBaseTempl<is_ad>
{
public:
  static InputParameters validParams();

  ScalarMaterialDamageTempl(const InputParameters & parameters);

protected:
  virtual void updateQpDamageIndex() override;

  ///@{ Material property that provides the damage index
  const GenericMaterialProperty<Real, is_ad> & _damage_property;
  ///@}

  using Material::_current_elem;
  using Material::_dt;
  using Material::_q_point;
  using Material::_qp;

  using ScalarDamageBaseTempl<is_ad>::_damage_index;
  using ScalarDamageBaseTempl<is_ad>::_base_name;
};

typedef ScalarMaterialDamageTempl<false> ScalarMaterialDamage;
typedef ScalarMaterialDamageTempl<true> ADScalarMaterialDamage;
