//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PorousFlowExponentialDecay.h"

#include "MooseVariable.h"

registerMooseObject("PorousFlowApp", PorousFlowExponentialDecay);

InputParameters
PorousFlowExponentialDecay::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addCoupledVar("rate", 1.0, "Rate of exponential decay");
  params.addCoupledVar("reference", 0.0, "Reference value of the variable");
  params.addClassDescription("Residual = rate * (variable - reference).  Useful for modelling "
                             "exponential decay of a variable");
  return params;
}

PorousFlowExponentialDecay::PorousFlowExponentialDecay(const InputParameters & parameters)
  : Kernel(parameters), _rate(coupledValue("rate")), _reference(coupledValue("reference"))
{
}

Real
PorousFlowExponentialDecay::computeQpResidual()
{
  return _test[_i][_qp] * _rate[_qp] * (_u[_qp] - _reference[_qp]);
}

Real
PorousFlowExponentialDecay::computeQpJacobian()
{
  return _test[_i][_qp] * _rate[_qp] * _phi[_j][_qp];
}
