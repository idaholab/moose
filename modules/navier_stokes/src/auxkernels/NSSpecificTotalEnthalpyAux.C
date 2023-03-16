//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// Navier-Stokes includes
#include "NSSpecificTotalEnthalpyAux.h"
#include "NS.h"

registerMooseObject("NavierStokesApp", NSSpecificTotalEnthalpyAux);

InputParameters
NSSpecificTotalEnthalpyAux::validParams()
{
  InputParameters params = AuxKernel::validParams();

  params.addClassDescription("Nodal auxiliary variable, for computing enthalpy at the nodes.");
  // Mark variables as required
  params.addRequiredCoupledVar(NS::density, "density");
  params.addRequiredCoupledVar(NS::total_energy_density, "total energy");
  params.addRequiredCoupledVar(NS::pressure, "pressure");

  return params;
}

NSSpecificTotalEnthalpyAux::NSSpecificTotalEnthalpyAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _rho(coupledValue(NS::density)),
    _rho_et(coupledValue(NS::total_energy_density)),
    _pressure(coupledValue(NS::pressure))
{
}

Real
NSSpecificTotalEnthalpyAux::computeValue()
{
  // H = (rho*E + P) / rho
  return (_rho_et[_qp] + _pressure[_qp]) / _rho[_qp];
}
