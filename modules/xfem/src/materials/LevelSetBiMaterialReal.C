//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetBiMaterialReal.h"

registerMooseObject("XFEMApp", LevelSetBiMaterialReal);

template <>
InputParameters
validParams<LevelSetBiMaterialReal>()
{
  InputParameters params = validParams<LevelSetBiMaterialBase>();
  params.addClassDescription(
      "Compute a Real material property for bi-materials problem (consisting of two "
      "different materials) defined by a level set function.");
  return params;
}

LevelSetBiMaterialReal::LevelSetBiMaterialReal(const InputParameters & parameters)
  : LevelSetBiMaterialBase(parameters),
    _bimaterial_material_prop(2),
    _material_prop(declareProperty<Real>(_base_name + _prop_name))
{
  _bimaterial_material_prop[0] = &getMaterialProperty<Real>(
      getParam<std::string>("levelset_positive_base") + "_" + _prop_name);
  _bimaterial_material_prop[1] = &getMaterialProperty<Real>(
      getParam<std::string>("levelset_negative_base") + "_" + _prop_name);
}

void
LevelSetBiMaterialReal::assignQpPropertiesForLevelSetPositive()
{
  _material_prop[_qp] = (*_bimaterial_material_prop[0])[_qp];
}

void
LevelSetBiMaterialReal::assignQpPropertiesForLevelSetNegative()
{
  _material_prop[_qp] = (*_bimaterial_material_prop[1])[_qp];
}
