//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NSImposedVelocityBC.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSImposedVelocityBC);

InputParameters
NSImposedVelocityBC::validParams()
{
  InputParameters params = NodalBC::validParams();
  params.addClassDescription("Impose Velocity BC.");
  params.addRequiredCoupledVar(NS::density, "density");
  params.addRequiredParam<Real>("desired_velocity", "");
  return params;
}

NSImposedVelocityBC::NSImposedVelocityBC(const InputParameters & parameters)
  : NodalBC(parameters),
    _rho(coupledValue(NS::density)),
    _desired_velocity(getParam<Real>("desired_velocity"))
{
}

Real
NSImposedVelocityBC::computeQpResidual()
{
  // Return the difference between the current momentum and the desired value
  // (rho*u) - rho*desired_velocity
  return _u[_qp] - (_rho[_qp] * _desired_velocity);
}
