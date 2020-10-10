//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SaturationPressureMaterial.h"
#include "TwoPhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", SaturationPressureMaterial);
registerMooseObject("FluidPropertiesApp", ADSaturationPressureMaterial);

template <bool is_ad>
InputParameters
SaturationPressureMaterialTempl<is_ad>::validParams()
{
  InputParameters params = Material::validParams();

  params.addClassDescription("Computes saturation pressure at some temperature.");

  params.addRequiredParam<MaterialPropertyName>("T", "Temperature material property");
  params.addRequiredParam<MaterialPropertyName>(
      "p_sat", "Name to give saturation pressure material property");

  params.addRequiredParam<UserObjectName>("fp_2phase",
                                          "Two-phase fluid properties user object name");

  return params;
}

template <bool is_ad>
SaturationPressureMaterialTempl<is_ad>::SaturationPressureMaterialTempl(
    const InputParameters & parameters)
  : Material(parameters),

    _T(getGenericMaterialProperty<Real, is_ad>("T")),
    _p_sat_name(getParam<MaterialPropertyName>("p_sat")),
    _p_sat(declareGenericProperty<Real, is_ad>(_p_sat_name)),

    _fp_2phase(getUserObject<TwoPhaseFluidProperties>("fp_2phase"))
{
}

template <bool is_ad>
void
SaturationPressureMaterialTempl<is_ad>::computeQpProperties()
{
  _p_sat[_qp] = _fp_2phase.p_sat(_T[_qp]);
}
