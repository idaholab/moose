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
  params.addRequiredCoupledVar("rho", "");
  params.addRequiredCoupledVar("rhoe", "");
  params.addRequiredCoupledVar("pressure", "");

  // Parameters with default values
  params.addRequiredParam<Real>("gamma", "Ratio of specific heats");

  return params;
}

NSEnthalpyAux::NSEnthalpyAux(const InputParameters & parameters) :
    AuxKernel(parameters),
    _rho(coupledValue("rho")),
    _rhoe(coupledValue("rhoe")),
    _pressure(coupledValue("pressure")),
    _gamma(getParam<Real>("gamma"))
{
}

Real
NSEnthalpyAux::computeValue()
{
  // H = (rho*E + P) / rho
  return (_rhoe[_qp] + _pressure[_qp]) / _rho[_qp];
}
