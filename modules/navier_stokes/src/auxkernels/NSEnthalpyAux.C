//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NSEnthalpyAux.h"
#include "NS.h"

template <>
InputParameters
validParams<NSEnthalpyAux>()
{
  InputParameters params = validParams<AuxKernel>();

  params.addClassDescription("Nodal auxiliary variable, for computing enthalpy at the nodes.");
  // Mark variables as required
  params.addRequiredCoupledVar(NS::density, "density");
  params.addRequiredCoupledVar(NS::total_energy, "total energy");
  params.addRequiredCoupledVar(NS::pressure, "pressure");

  return params;
}

NSEnthalpyAux::NSEnthalpyAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _rho(coupledValue(NS::density)),
    _rhoE(coupledValue(NS::total_energy)),
    _pressure(coupledValue(NS::pressure))
{
}

Real
NSEnthalpyAux::computeValue()
{
  // H = (rho*E + P) / rho
  return (_rhoE[_qp] + _pressure[_qp]) / _rho[_qp];
}
