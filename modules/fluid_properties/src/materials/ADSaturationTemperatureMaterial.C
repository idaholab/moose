//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADSaturationTemperatureMaterial.h"
#include "TwoPhaseFluidProperties.h"

registerMooseObject("FluidPropertiesApp", ADSaturationTemperatureMaterial);

InputParameters
ADSaturationTemperatureMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<MaterialPropertyName>("p", "Pressure material property");
  params.addRequiredParam<MaterialPropertyName>(
      "T_sat", "Name to give saturation temperature material property");

  params.addRequiredParam<UserObjectName>("fp_2phase",
                                          "Two-phase fluid properties user object name");

  params.addClassDescription("Computes saturation temperature at some pressure");

  return params;
}

ADSaturationTemperatureMaterial::ADSaturationTemperatureMaterial(const InputParameters & parameters)
  : Material(parameters),

    _p(getADMaterialProperty<Real>("p")),
    _T_sat_name(getParam<MaterialPropertyName>("T_sat")),
    _T_sat(declareADProperty<Real>(_T_sat_name)),

    _fp_2phase(getUserObject<TwoPhaseFluidProperties>("fp_2phase"))
{
}

void
ADSaturationTemperatureMaterial::computeQpProperties()
{
  _T_sat[_qp] = _fp_2phase.T_sat(_p[_qp]);
}
