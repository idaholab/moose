//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneDMomentumMassFlowRateTemperatureBC.h"

registerMooseObject("ThermalHydraulicsApp", OneDMomentumMassFlowRateTemperatureBC);

InputParameters
OneDMomentumMassFlowRateTemperatureBC::validParams()
{
  InputParameters params = OneDNodalBC::validParams();
  params.addRequiredParam<Real>("m_dot", "The specified mass flow rate value.");

  params.declareControllable("m_dot");

  return params;
}

OneDMomentumMassFlowRateTemperatureBC::OneDMomentumMassFlowRateTemperatureBC(
    const InputParameters & parameters)
  : OneDNodalBC(parameters), _m_dot(getParam<Real>("m_dot"))
{
}

Real
OneDMomentumMassFlowRateTemperatureBC::computeQpResidual()
{
  return _u[_qp] - _m_dot;
}

Real
OneDMomentumMassFlowRateTemperatureBC::computeQpJacobian()
{
  return 1;
}
