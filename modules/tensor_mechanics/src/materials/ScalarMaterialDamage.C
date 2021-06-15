//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ScalarMaterialDamage.h"

registerMooseObject("TensorMechanicsApp", ScalarMaterialDamage);
registerMooseObject("TensorMechanicsApp", ADScalarMaterialDamage);

template <bool is_ad>
InputParameters
ScalarMaterialDamageTempl<is_ad>::validParams()
{
  InputParameters params = ScalarDamageBaseTempl<is_ad>::validParams();
  params.addClassDescription(
      "Scalar damage model for which the damage is prescribed by another material");
  params.addRequiredParam<MaterialPropertyName>("damage_index",
                                                "Name of the material property containing the "
                                                "damage index, which goes from 0 (undamaged) to 1 "
                                                "(fully damaged)");
  return params;
}

template <bool is_ad>
ScalarMaterialDamageTempl<is_ad>::ScalarMaterialDamageTempl(const InputParameters & parameters)
  : ScalarDamageBaseTempl<is_ad>(parameters),
    _damage_property(this->template getGenericMaterialProperty<Real, is_ad>("damage_index"))
{
}

template <bool is_ad>
void
ScalarMaterialDamageTempl<is_ad>::updateQpDamageIndex()
{
  _damage_index[_qp] = _damage_property[_qp];

  if (MooseUtils::absoluteFuzzyLessThan(_damage_index[_qp], 0.0) ||
      MooseUtils::absoluteFuzzyGreaterThan(_damage_index[_qp], 1.0))
    mooseError(_base_name + "damage_index ",
               "must be between 0 and 1. Current value is: ",
               _damage_index[_qp]);
}

template class ScalarMaterialDamageTempl<false>;
template class ScalarMaterialDamageTempl<true>;
