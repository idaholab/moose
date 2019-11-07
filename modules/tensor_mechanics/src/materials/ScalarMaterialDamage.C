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

defineLegacyParams(ScalarMaterialDamage);

InputParameters
ScalarMaterialDamage::validParams()
{
  InputParameters params = ScalarDamageBase::validParams();
  params.addClassDescription(
      "Scalar damage model for which the damage is prescribed by another material");
  params.addRequiredParam<MaterialPropertyName>("damage_index",
                                                "Name of the material property containing the "
                                                "damage index, which goes from 0 (undamaged) to 1 "
                                                "(fully damaged)");
  return params;
}

ScalarMaterialDamage::ScalarMaterialDamage(const InputParameters & parameters)
  : ScalarDamageBase(parameters),
    _damage_property(
        getMaterialPropertyByName<Real>(getParam<MaterialPropertyName>("damage_index")))
{
}

void
ScalarMaterialDamage::updateQpDamageIndex()
{
  _damage_index[_qp] = _damage_property[_qp];

  if (MooseUtils::absoluteFuzzyLessThan(_damage_index[_qp], 0.0) ||
      MooseUtils::absoluteFuzzyGreaterThan(_damage_index[_qp], 1.0))
    mooseError(_base_name + "damage_index ",
               "must be between 0 and 1. Current value is: ",
               _damage_index[_qp]);
}
