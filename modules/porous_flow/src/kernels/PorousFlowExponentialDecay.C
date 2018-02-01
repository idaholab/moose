/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#include "PorousFlowExponentialDecay.h"

// MOOSE includes
#include "MooseVariable.h"

template <>
InputParameters
validParams<PorousFlowExponentialDecay>()
{
  InputParameters params = validParams<Kernel>();
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
