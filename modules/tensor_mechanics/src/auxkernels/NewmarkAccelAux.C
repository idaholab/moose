//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NewmarkAccelAux.h"

registerMooseObject("TensorMechanicsApp", NewmarkAccelAux);

InputParameters
NewmarkAccelAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Computes the current acceleration using the Newmark method.");
  params.addRequiredCoupledVar("displacement", "displacement variable");
  params.addRequiredCoupledVar("velocity", "velocity variable");
  params.addRequiredParam<Real>("beta", "beta parameter for Newmark method");
  return params;
}

NewmarkAccelAux::NewmarkAccelAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _disp_old(coupledValueOld("displacement")),
    _disp(coupledValue("displacement")),
    _vel_old(coupledValueOld("velocity")),
    _u_old(uOld()),
    _beta(getParam<Real>("beta"))
{
}

Real
NewmarkAccelAux::computeValue()
{
  if (!isNodal())
    mooseError("must run on a nodal variable");

  Real accel_old = _u_old[_qp];
  if (_dt == 0)
    return accel_old;

  // Calculates acceeleration using Newmark time integration method
  return 1.0 / _beta *
         ((_disp[_qp] - _disp_old[_qp]) / (_dt * _dt) - _vel_old[_qp] / _dt -
          accel_old * (0.5 - _beta));
}
