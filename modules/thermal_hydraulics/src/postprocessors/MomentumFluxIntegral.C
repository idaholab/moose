//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "MomentumFluxIntegral.h"

registerMooseObject("ThermalHydraulicsApp", MomentumFluxIntegral);

InputParameters
MomentumFluxIntegral::validParams()
{
  InputParameters params = SideIntegralPostprocessor::validParams();
  params.addRequiredCoupledVar("arhouA", "Momentum equation variable");
  params.addRequiredCoupledVar("vel", "Velocity");
  params.addRequiredCoupledVar("p", "Pressure");
  params.addRequiredCoupledVar("A", "Area");
  params.addCoupledVar("alpha", 1.0, "Volume fraction (two-phase only)");
  return params;
}

MomentumFluxIntegral::MomentumFluxIntegral(const InputParameters & parameters)
  : SideIntegralPostprocessor(parameters),
    _arhouA(coupledValue("arhouA")),
    _velocity(coupledValue("vel")),
    _pressure(coupledValue("p")),
    _area(coupledValue("A")),
    _alpha(coupledValue("alpha"))
{
}

void
MomentumFluxIntegral::threadJoin(const UserObject & y)
{
  const MomentumFluxIntegral & pps = static_cast<const MomentumFluxIntegral &>(y);
  _integral_value += pps._integral_value;
}

Real
MomentumFluxIntegral::computeQpIntegral()
{
  return _arhouA[_qp] * _velocity[_qp] + _alpha[_qp] * _pressure[_qp] * _area[_qp];
}
