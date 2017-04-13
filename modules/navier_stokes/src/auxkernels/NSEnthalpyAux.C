/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

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
