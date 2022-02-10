//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneDAreaTimesConstantBC.h"

registerMooseObject("ThermalHydraulicsApp", OneDAreaTimesConstantBC);

InputParameters
OneDAreaTimesConstantBC::validParams()
{
  InputParameters params = OneDNodalBC::validParams();
  params.addRequiredParam<Real>("value", "The constant value used.");
  params.addRequiredCoupledVar("A", "Area");
  params.declareControllable("value");
  return params;
}

OneDAreaTimesConstantBC::OneDAreaTimesConstantBC(const InputParameters & parameters)
  : OneDNodalBC(parameters), _value(getParam<Real>("value")), _area(coupledValue("A"))
{
}

Real
OneDAreaTimesConstantBC::computeQpResidual()
{
  return _u[_qp] - _area[_qp] * _value;
}
