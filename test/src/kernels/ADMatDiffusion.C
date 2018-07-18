//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#include "ADMatDiffusion.h"

registerMooseObject("MooseTestApp", ADMatDiffusion);

template <>
InputParameters
validParams<ADMatDiffusion>()
{
  InputParameters params = validParams<ADKernel>();
  params.addRequiredParam<MaterialPropertyName>(
      "prop_name", "the name of the material property we are going to use");

  MooseEnum prop_state("current old older", "current");
  params.addParam<MooseEnum>(
      "prop_state", prop_state, "Declares which property state we should retrieve");
  return params;
}

ADMatDiffusion::ADMatDiffusion(const InputParameters & parameters) : ADKernel(parameters)
{
  MooseEnum prop_state = getParam<MooseEnum>("prop_state");

  if (prop_state == "current")
    _diff = &getMaterialProperty<ADReal>("prop_name");
  else if (prop_state == "old")
    _diff = &getMaterialPropertyOld<ADReal>("prop_name");
  else if (prop_state == "older")
    _diff = &getMaterialPropertyOlder<ADReal>("prop_name");
}

ADReal
ADMatDiffusion::computeQpResidual()
{
  return (*_diff)[_qp] * (_grad_test[_i][_qp] * _grad_u[_qp]);
}
