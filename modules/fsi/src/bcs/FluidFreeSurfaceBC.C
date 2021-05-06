//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidFreeSurfaceBC.h"

registerMooseObject("FsiApp", FluidFreeSurfaceBC);

InputParameters
FluidFreeSurfaceBC::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addClassDescription("Applies a mixed Dirichlet-Neumann BC on the fluid surface.");
  params.addRangeCheckedParam<Real>(
      "alpha", 0.1, "alpha>0", "Inverse of the acceleration due to gravity.");
  return params;
}

FluidFreeSurfaceBC::FluidFreeSurfaceBC(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _alpha(getParam<Real>("alpha")),
    _u_dotdot(dotDot()),
    _du_dotdot_du(dotDotDu())
{
}

Real
FluidFreeSurfaceBC::computeQpResidual()
{
  return _test[_i][_qp] * _alpha * _u_dotdot[_qp];
}

Real
FluidFreeSurfaceBC::computeQpJacobian()
{
  return _test[_i][_qp] * _alpha * _du_dotdot_du[_qp] * _phi[_j][_qp];
}
