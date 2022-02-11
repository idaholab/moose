//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneDMassMassFlowRateTemperatureBC.h"

registerMooseObject("ThermalHydraulicsApp", OneDMassMassFlowRateTemperatureBC);

InputParameters
OneDMassMassFlowRateTemperatureBC::validParams()
{
  InputParameters params = OneDIntegratedBC::validParams();
  params.addRequiredParam<Real>("m_dot", "The specified mass flow rate value.");

  params.declareControllable("m_dot");

  return params;
}

OneDMassMassFlowRateTemperatureBC::OneDMassMassFlowRateTemperatureBC(
    const InputParameters & parameters)
  : OneDIntegratedBC(parameters), _m_dot(getParam<Real>("m_dot"))
{
}

Real
OneDMassMassFlowRateTemperatureBC::computeQpResidual()
{
  return _m_dot * _normal * _test[_i][_qp];
}

Real
OneDMassMassFlowRateTemperatureBC::computeQpJacobian()
{
  return 0.;
}
