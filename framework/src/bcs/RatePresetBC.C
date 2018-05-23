//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "RatePresetBC.h"
#include "Function.h"

registerMooseObject("MooseApp", RatePresetBC);

template <>
InputParameters
validParams<RatePresetBC>()
{
  InputParameters params = validParams<PresetNodalBC>();
  params.addRequiredParam<Real>("rate", "Value of the rate.");
  params.declareControllable("rate");
  params.addClassDescription(
      "The same as RateDirichletBC except the value is applied before the solve begins");
  return params;
}

RatePresetBC::RatePresetBC(const InputParameters & parameters)
  : PresetNodalBC(parameters), _rate(getParam<Real>("rate")), _u_old(_var.dofValuesOld())
{
}

Real
RatePresetBC::computeQpValue()
{
  return _u_old[_qp] + _rate * _dt;
}
