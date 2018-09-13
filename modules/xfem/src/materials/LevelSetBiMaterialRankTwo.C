/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "LevelSetBiMaterialRankTwo.h"

registerMooseObject("XFEMApp", LevelSetBiMaterialRankTwo);

template <>
InputParameters
validParams<LevelSetBiMaterialRankTwo>()
{
  InputParameters params = validParams<LevelSetBiMaterialBase>();
  params.addClassDescription(
      "Compute a RankTwoTensor material property for bi-materials problem (consisting of two "
      "different materials) defined by a level set function.");
  return params;
}

LevelSetBiMaterialRankTwo::LevelSetBiMaterialRankTwo(const InputParameters & parameters)
  : LevelSetBiMaterialBase(parameters),
    _bimaterial_material_prop(2),
    _material_prop(declareProperty<RankTwoTensor>(_base_name + _prop_name))
{
  _bimaterial_material_prop[0] = &getMaterialProperty<RankTwoTensor>(
      getParam<std::string>("levelset_positive_base") + "_" + _prop_name);
  _bimaterial_material_prop[1] = &getMaterialProperty<RankTwoTensor>(
      getParam<std::string>("levelset_negative_base") + "_" + _prop_name);
}

void
LevelSetBiMaterialRankTwo::assignQpPropertiesForLevelSetPositive()
{
  _material_prop[_qp] = (*_bimaterial_material_prop[0])[_qp];
}

void
LevelSetBiMaterialRankTwo::assignQpPropertiesForLevelSetNegative()
{
  _material_prop[_qp] = (*_bimaterial_material_prop[1])[_qp];
}
