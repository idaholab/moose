//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NSVelocityAux.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSVelocityAux);

InputParameters
NSVelocityAux::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Velocity auxiliary value.");
  params.addRequiredCoupledVar(NS::density, "Density (conserved form)");
  params.addRequiredCoupledVar("momentum", "Momentum (conserved form)");
  params.addParam<UserObjectName>(
      "fluid_properties", "", "The name of the user object for fluid properties");
  return params;
}

NSVelocityAux::NSVelocityAux(const InputParameters & parameters)
  : AuxKernel(parameters), _rho(coupledValue(NS::density)), _momentum(coupledValue("momentum"))
{
}

Real
NSVelocityAux::computeValue()
{
  return _momentum[_qp] / _rho[_qp];
}
