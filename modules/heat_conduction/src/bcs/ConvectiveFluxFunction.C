/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "ConvectiveFluxFunction.h"

#include "Function.h"

template <>
InputParameters
validParams<ConvectiveFluxFunction>()
{
  InputParameters params = validParams<IntegratedBC>();
  params.addRequiredParam<FunctionName>("T_infinity", "Function describing far-field temperature");
  params.addRequiredParam<Real>("coefficient", "Heat transfer coefficient");
  params.addParam<FunctionName>("coefficient_function", "Heat transfer coefficient function");

  return params;
}

ConvectiveFluxFunction::ConvectiveFluxFunction(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _T_infinity(getFunction("T_infinity")),
    _coefficient(getParam<Real>("coefficient")),
    _coef_func(isParamValid("coefficient_function") ? &getFunction("coefficient_function") : NULL)
{
}

Real
ConvectiveFluxFunction::computeQpResidual()
{
  const Real coef(_coefficient * (_coef_func ? _coef_func->value(_t, _q_point[_qp]) : 1));
  return _test[_i][_qp] * coef * (_u[_qp] - _T_infinity.value(_t, _q_point[_qp]));
}

Real
ConvectiveFluxFunction::computeQpJacobian()
{
  const Real coef(_coefficient * (_coef_func ? _coef_func->value(_t, _q_point[_qp]) : 1));
  return _test[_i][_qp] * coef * _phi[_j][_qp];
}
