//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADPenaltyInterfaceDiffusion.h"

registerMooseObject("MooseApp", ADPenaltyInterfaceDiffusion);

InputParameters
ADPenaltyInterfaceDiffusion::validParams()
{
  InputParameters params = ADInterfaceKernel::validParams();
  params.addRequiredParam<Real>(
      "penalty", "The penalty that penalizes jump between primary and neighbor variables.");
  params.addParam<MaterialPropertyName>(
      "jump_prop_name", "the name of the material property that calculates the jump.");
  params.addClassDescription(
      "A penalty-based interface condition that forces"
      "the continuity of variables and the flux equivalence across an interface.");
  return params;
}

ADPenaltyInterfaceDiffusion::ADPenaltyInterfaceDiffusion(const InputParameters & parameters)
  : ADInterfaceKernel(parameters),
    _penalty(getParam<Real>("penalty")),
    _jump(isParamValid("jump_prop_name") ? &getADMaterialProperty<Real>("jump_prop_name") : nullptr)
{
}

ADReal
ADPenaltyInterfaceDiffusion::computeQpResidual(Moose::DGResidualType type)
{
  ADReal r = 0;

  ADReal jump_value = 0;

  if (_jump != nullptr)
    jump_value = (*_jump)[_qp];
  else
    jump_value = _u[_qp] - _neighbor_value[_qp];

  switch (type)
  {
    case Moose::Element:
      r = _test[_i][_qp] * _penalty * jump_value;
      break;

    case Moose::Neighbor:
      r = _test_neighbor[_i][_qp] * -_penalty * jump_value;
      break;
  }

  return r;
}
