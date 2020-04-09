//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PresetVelocity.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", PresetVelocity);

InputParameters
PresetVelocity::validParams()
{
  InputParameters params = DirichletBCBase::validParams();

  params.addParam<Real>(
      "velocity", 1, "Value of the velocity.  Used as scale factor if function is given.");
  params.addParam<FunctionName>("function", "1", "Function describing the velocity.");

  // Forcefully preset the BC
  params.set<bool>("preset") = true;
  params.suppressParameter<bool>("preset");

  return params;
}

PresetVelocity::PresetVelocity(const InputParameters & parameters)
  : DirichletBCBase(parameters),
    _u_old(valueOld()),
    _velocity(parameters.get<Real>("velocity")),
    _function(getFunction("function"))
{
}

Real
PresetVelocity::computeQpValue()
{
  Real v2 = _function.value(_t, *_current_node);
  Real v1 = _function.value(_t - _dt, *_current_node);
  Real vel = _velocity * 0.5 * (v1 + v2);

  return _u_old[_qp] + _dt * vel;
}
