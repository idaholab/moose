//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetBiMaterialRankFour.h"

registerMooseObjectReplaced("XFEMApp",
                            LevelSetBiMaterialRankFour,
                            "01/01/2022 00:00",
                            XFEMCutSwitchingMaterialRankFourTensor);
registerMooseObjectReplaced("XFEMApp",
                            ADLevelSetBiMaterialRankFour,
                            "01/01/2022 00:00",
                            ADXFEMCutSwitchingMaterialRankFourTensor);

template <bool is_ad>
InputParameters
LevelSetBiMaterialRankFourTempl<is_ad>::validParams()
{
  InputParameters params = LevelSetBiMaterialBase::validParams();
  params.addClassDescription(
      "Compute a RankFourTensor material property for bi-materials problem (consisting of two "
      "different materials) defined by a level set function.");
  return params;
}

template <bool is_ad>
LevelSetBiMaterialRankFourTempl<is_ad>::LevelSetBiMaterialRankFourTempl(
    const InputParameters & parameters)
  : LevelSetBiMaterialBase(parameters),
    _bimaterial_material_prop(2),
    _material_prop(declareGenericProperty<RankFourTensor, is_ad>(_base_name + _prop_name))
{
  _bimaterial_material_prop[0] = &getGenericMaterialProperty<RankFourTensor, is_ad>(
      getParam<std::string>("levelset_positive_base") + "_" + _prop_name);
  _bimaterial_material_prop[1] = &getGenericMaterialProperty<RankFourTensor, is_ad>(
      getParam<std::string>("levelset_negative_base") + "_" + _prop_name);
}

template <bool is_ad>
void
LevelSetBiMaterialRankFourTempl<is_ad>::assignQpPropertiesForLevelSetPositive()
{
  _material_prop[_qp] = (*_bimaterial_material_prop[0])[_qp];
}

template <bool is_ad>
void
LevelSetBiMaterialRankFourTempl<is_ad>::assignQpPropertiesForLevelSetNegative()
{
  _material_prop[_qp] = (*_bimaterial_material_prop[1])[_qp];
}

template class LevelSetBiMaterialRankFourTempl<false>;
template class LevelSetBiMaterialRankFourTempl<true>;
