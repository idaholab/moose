//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "EnergyFluxIntegral.h"

registerMooseObject("ThermalHydraulicsApp", EnergyFluxIntegral);

InputParameters
EnergyFluxIntegral::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredCoupledVar("arhouA", "alpha*rho*u*A");
  params.addRequiredCoupledVar("H", "Specific total enthalpy");
  return params;
}

EnergyFluxIntegral::EnergyFluxIntegral(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _arhouA(coupledValue("arhouA")),
    _enthalpy(coupledValue("H"))
{
}

void
EnergyFluxIntegral::threadJoin(const UserObject & y)
{
  const EnergyFluxIntegral & pps = static_cast<const EnergyFluxIntegral &>(y);
  _integral_value += pps._integral_value;
}

Real
EnergyFluxIntegral::computeQpIntegral()
{
  return _arhouA[_qp] * _enthalpy[_qp];
}
