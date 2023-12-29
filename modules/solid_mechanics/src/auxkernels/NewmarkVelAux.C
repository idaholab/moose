//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NewmarkVelAux.h"

registerMooseObject("TensorMechanicsApp", NewmarkVelAux);

InputParameters
NewmarkVelAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Calculates the current velocity using Newmark method.");
  params.addRequiredCoupledVar("acceleration", "acceleration variable");
  params.addRequiredParam<Real>("gamma", "gamma parameter for Newmark method");
  return params;
}

NewmarkVelAux::NewmarkVelAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _accel_old(coupledValueOld("acceleration")),
    _accel(coupledValue("acceleration")),
    _u_old(uOld()),
    _gamma(getParam<Real>("gamma"))
{
}

Real
NewmarkVelAux::computeValue()
{
  Real vel_old = _u_old[_qp];
  if (!isNodal())
    mooseError("must run on a nodal variable");
  // Calculates Velocity using Newmark time integration scheme
  return vel_old + (_dt * (1 - _gamma)) * _accel_old[_qp] + _gamma * _dt * _accel[_qp];
}
