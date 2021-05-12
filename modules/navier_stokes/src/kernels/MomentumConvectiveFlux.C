//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MomentumConvectiveFlux.h"

registerMooseObject("NavierStokesApp", MomentumConvectiveFlux);

InputParameters
MomentumConvectiveFlux::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredCoupledVar("vel_x", "");
  params.addCoupledVar("vel_y", "");
  params.addCoupledVar("vel_z", "");
  params.addClassDescription(
      "Implements the advective term of the Navier Stokes momentum equation.");

  return params;
}

MomentumConvectiveFlux::MomentumConvectiveFlux(const InputParameters & parameters)
  : Kernel(parameters),
    _vel_x(coupledValue("vel_x")),
    _vel_y(isCoupled("vel_y") ? coupledValue("vel_y") : _zero),
    _vel_z(isCoupled("vel_z") ? coupledValue("vel_z") : _zero)
{
}

Real
MomentumConvectiveFlux::computeQpResidual()
{
  RealVectorValue vel_vec(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]);
  return -_u[_qp] * (vel_vec * _grad_test[_i][_qp]);
}

Real
MomentumConvectiveFlux::computeQpJacobian()
{
  return 0.;
}
