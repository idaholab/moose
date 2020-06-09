//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADLevelSetBiMaterialRankFour.h"

registerMooseObject("XFEMApp", ADLevelSetBiMaterialRankFour);

InputParameters
ADLevelSetBiMaterialRankFour::validParams()
{
  InputParameters params = ADLevelSetBiMaterialBase::validParams();
  params.addClassDescription(
      "Compute an AD RankFourTensor material property for bi-materials problem (consisting of two "
      "different materials) defined by a level set function.");
  return params;
}

ADLevelSetBiMaterialRankFour::ADLevelSetBiMaterialRankFour(const InputParameters & parameters)
  : ADLevelSetBiMaterialBase(parameters),
    _bimaterial_material_prop(2),
    _material_prop(declareADProperty<RankFourTensor>(_base_name + _prop_name))
{
  _bimaterial_material_prop[0] = &getADMaterialProperty<RankFourTensor>(
      getParam<std::string>("levelset_positive_base") + "_" + _prop_name);
  _bimaterial_material_prop[1] = &getADMaterialProperty<RankFourTensor>(
      getParam<std::string>("levelset_negative_base") + "_" + _prop_name);
}

void
ADLevelSetBiMaterialRankFour::assignQpPropertiesForLevelSetPositive()
{
  _material_prop[_qp] = (*_bimaterial_material_prop[0])[_qp];
}

void
ADLevelSetBiMaterialRankFour::assignQpPropertiesForLevelSetNegative()
{
  _material_prop[_qp] = (*_bimaterial_material_prop[1])[_qp];
}
