//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADWallFrictionFunctionMaterial.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADWallFrictionFunctionMaterial);

InputParameters
ADWallFrictionFunctionMaterial::validParams()
{
  InputParameters params = Material::validParams();

  params.addRequiredParam<MaterialPropertyName>("f_D", "Darcy friction factor material property");

  params.addRequiredParam<FunctionName>("function", "Darcy friction factor function");

  return params;
}

ADWallFrictionFunctionMaterial::ADWallFrictionFunctionMaterial(const InputParameters & parameters)
  : Material(parameters),

    _function(getFunction("function")),

    _f_D_name(getParam<MaterialPropertyName>("f_D")),
    _f_D(declareADProperty<Real>(_f_D_name))
{
}

void
ADWallFrictionFunctionMaterial::computeQpProperties()
{
  _f_D[_qp] = _function.value(_t, _q_point[_qp]);
}
