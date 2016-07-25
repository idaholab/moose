/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NSEnthalpyAux.h"

template<>
InputParameters validParams<NSEnthalpyAux>()
{
  InputParameters params = validParams<AuxKernel>();

  // Mark variables as required
  params.addRequiredCoupledVar("rho", "density");
  params.addRequiredCoupledVar("rhoE", "total energy");
  params.addRequiredCoupledVar("pressure", "pressure");

  return params;
}

NSEnthalpyAux::NSEnthalpyAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _rho(coupledValue("rho")),
    _rhoE(coupledValue("rhoE")),
    _pressure(coupledValue("pressure"))
{
}

Real
NSEnthalpyAux::computeValue()
{
  // H = (rho*E + P) / rho
  return (_rhoE[_qp] + _pressure[_qp]) / _rho[_qp];
}
