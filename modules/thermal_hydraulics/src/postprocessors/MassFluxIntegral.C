//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MassFluxIntegral.h"

registerMooseObject("ThermalHydraulicsApp", MassFluxIntegral);

InputParameters
MassFluxIntegral::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredCoupledVar("arhouA", "Momentum equation variable");
  return params;
}

MassFluxIntegral::MassFluxIntegral(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters), _arhouA(coupledValue("arhouA"))
{
}

void
MassFluxIntegral::threadJoin(const UserObject & y)
{
  const MassFluxIntegral & pps = static_cast<const MassFluxIntegral &>(y);
  _integral_value += pps._integral_value;
}

Real
MassFluxIntegral::computeQpIntegral()
{
  return _arhouA[_qp];
}
