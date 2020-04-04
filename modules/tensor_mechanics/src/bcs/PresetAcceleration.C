//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PresetAcceleration.h"
#include "Function.h"

registerMooseObject("TensorMechanicsApp", PresetAcceleration);

InputParameters
PresetAcceleration::validParams()
{
  InputParameters params = DirichletBCBase::validParams();
  params.addClassDescription("Prescribe acceleration on a given boundary in a given direction");

  params.addParam<Real>("scale_factor", 1, "Scale factor if function is given.");
  params.addParam<FunctionName>("function", "1", "Function describing the velocity.");
  params.addRequiredCoupledVar("velocity", "The velocity variable.");
  params.addRequiredCoupledVar("acceleration", "The acceleration variable.");
  params.addRequiredParam<Real>("beta", "beta parameter for Newmark time integration.");

  // Forcefully preset the BC
  params.set<bool>("preset") = true;
  params.suppressParameter<bool>("preset");

  return params;
}

PresetAcceleration::PresetAcceleration(const InputParameters & parameters)
  : DirichletBCBase(parameters),
    _u_old(valueOld()),
    _scale_factor(parameters.get<Real>("scale_factor")),
    _function(getFunction("function")),
    _vel_old(coupledValueOld("velocity")),
    _accel_old(coupledValueOld("acceleration")),
    _beta(getParam<Real>("beta"))
{
}

Real
PresetAcceleration::computeQpValue()
{
  Real accel = _function.value(_t, *_current_node);

  // Integrate acceleration using Newmark time integration to get displacement
  return _u_old[_qp] + _dt * _vel_old[_qp] +
         ((0.5 - _beta) * _accel_old[_qp] + _beta * accel) * _dt * _dt;
}
