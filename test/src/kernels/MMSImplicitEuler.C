//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MMSImplicitEuler.h"

registerMooseObject("MooseTestApp", MMSImplicitEuler);

InputParameters
MMSImplicitEuler::validParams()
{
  InputParameters params = TimeKernel::validParams();
  return params;
}

MMSImplicitEuler::MMSImplicitEuler(const InputParameters & parameters)
  : TimeKernel(parameters), _u_old(_var.slnOld())
{
}

Real
MMSImplicitEuler::computeQpResidual()
{
  return _test[_i][_qp] * ((_u[_qp] - _u_old[_qp]) / _dt);
}

Real
MMSImplicitEuler::computeQpJacobian()
{
  return _test[_i][_qp] * _phi[_j][_qp] / _dt;
}
