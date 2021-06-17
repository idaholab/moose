//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "LevelSetBiMaterialReal.h"

registerMooseObjectReplaced("XFEMApp",
                            LevelSetBiMaterialReal,
                            "01/01/2022 00:00",
                            XFEMCutSwitchingMaterialReal);
registerMooseObjectReplaced("XFEMApp",
                            ADLevelSetBiMaterialReal,
                            "01/01/2022 00:00",
                            ADXFEMCutSwitchingMaterialReal);

template <bool is_ad>
InputParameters
LevelSetBiMaterialRealTempl<is_ad>::validParams()
{
  InputParameters params = LevelSetBiMaterialBase::validParams();
  params.addClassDescription(
      "Compute a Real material property for bi-materials problem (consisting of two "
      "different materials) defined by a level set function.");
  return params;
}

template <bool is_ad>
LevelSetBiMaterialRealTempl<is_ad>::LevelSetBiMaterialRealTempl(const InputParameters & parameters)
  : LevelSetBiMaterialBase(parameters),
    _bimaterial_material_prop(2),
    _material_prop(declareGenericProperty<Real, is_ad>(_base_name + _prop_name))
{
  _bimaterial_material_prop[0] = &getGenericMaterialProperty<Real, is_ad>(
      getParam<std::string>("levelset_positive_base") + "_" + _prop_name);
  _bimaterial_material_prop[1] = &getGenericMaterialProperty<Real, is_ad>(
      getParam<std::string>("levelset_negative_base") + "_" + _prop_name);
}

template <bool is_ad>
void
LevelSetBiMaterialRealTempl<is_ad>::assignQpPropertiesForLevelSetPositive()
{
  _material_prop[_qp] = (*_bimaterial_material_prop[0])[_qp];
}

template <bool is_ad>
void
LevelSetBiMaterialRealTempl<is_ad>::assignQpPropertiesForLevelSetNegative()
{
  _material_prop[_qp] = (*_bimaterial_material_prop[1])[_qp];
}

template class LevelSetBiMaterialRealTempl<false>;
template class LevelSetBiMaterialRealTempl<true>;
