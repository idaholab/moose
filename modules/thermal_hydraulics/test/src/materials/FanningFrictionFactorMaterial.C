//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FanningFrictionFactorMaterial.h"

registerMooseObject("ThermalHydraulicsTestApp", FanningFrictionFactorMaterial);

InputParameters
FanningFrictionFactorMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<MaterialPropertyName>(
      "f_F", "Name to give Fanning friction factor material property");
  params.addRequiredParam<MaterialPropertyName>("f_D", "Darcy friction factor material property");

  params.addClassDescription("Computes Fanning friction factor from Darcy friction factor");

  return params;
}

FanningFrictionFactorMaterial::FanningFrictionFactorMaterial(const InputParameters & parameters)
  : Material(parameters),

    _f_D(getMaterialProperty<Real>("f_D")),
    _f_F(declareProperty<Real>(getParam<MaterialPropertyName>("f_F")))
{
}

void
FanningFrictionFactorMaterial::computeQpProperties()
{
  _f_F[_qp] = 0.25 * _f_D[_qp];
}
