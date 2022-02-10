//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "OneD3EqnEnergyHeatSource.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", OneD3EqnEnergyHeatSource);

InputParameters
OneD3EqnEnergyHeatSource::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addRequiredParam<FunctionName>("q", "Volumetric heat source");
  params.addRequiredCoupledVar("A", "Cross sectional area");
  params.addClassDescription("Volumetric heat source for 1-phase flow channel");
  return params;
}

OneD3EqnEnergyHeatSource::OneD3EqnEnergyHeatSource(const InputParameters & parameters)
  : Kernel(parameters), _q(getFunction("q")), _A(coupledValue("A"))
{
}

Real
OneD3EqnEnergyHeatSource::computeQpResidual()
{
  return -_q.value(_t, _q_point[_qp]) * _A[_qp] * _test[_i][_qp];
}

Real
OneD3EqnEnergyHeatSource::computeQpJacobian()
{
  return 0.;
}

Real
OneD3EqnEnergyHeatSource::computeQpOffDiagJacobian(unsigned int /*jvar*/)
{
  return 0.;
}
