//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassConvectiveFlux.h"

registerMooseObject("NavierStokesApp", MassConvectiveFlux);

InputParameters
MassConvectiveFlux::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredCoupledVar("vel_x", "x-component of the velocity vector");
  params.addCoupledVar("vel_y", "y-component of the velocity vector");
  params.addCoupledVar("vel_z", "z-component of the velocity vector");
  params.addClassDescription("Implements the advection term for the Navier Stokes mass equation.");

  return params;
}

MassConvectiveFlux::MassConvectiveFlux(const InputParameters & parameters)
  : Kernel(parameters),
    _vel_x(coupledValue("vel_x")),
    _vel_y(isCoupled("vel_y") ? coupledValue("vel_y") : _zero),
    _vel_z(isCoupled("vel_z") ? coupledValue("vel_z") : _zero)
{
}

Real
MassConvectiveFlux::computeQpResidual()
{
  RealVectorValue vel_vec(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]);
  return -_u[_qp] * vel_vec * _grad_test[_i][_qp];
}

Real
MassConvectiveFlux::computeQpJacobian()
{
  RealVectorValue vel_vec(_vel_x[_qp], _vel_y[_qp], _vel_z[_qp]);
  return -_phi[_j][_qp] * vel_vec * _grad_test[_i][_qp];
}
