//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ConvectiveFluxFunction.h"

#include "Function.h"

registerMooseObject("HeatConductionApp", ConvectiveFluxFunction);

InputParameters
ConvectiveFluxFunction::validParams()
{
  InputParameters params = IntegratedBC::validParams();
  params.addRequiredParam<FunctionName>("T_infinity", "Function describing far-field temperature");
  params.addRequiredParam<FunctionName>("coefficient",
                                        "Function describing heat transfer coefficient");
  MooseEnum coef_func_type("TIME_AND_POSITION TEMPERATURE", "TIME_AND_POSITION");
  params.addParam<MooseEnum>(
      "coefficient_function_type",
      coef_func_type,
      "Type of function for heat transfer coefficient provided in 'coefficient' parameter");
  params.addClassDescription(
      "Determines boundary value by fluid heat transfer coefficient and far-field temperature");

  return params;
}

ConvectiveFluxFunction::ConvectiveFluxFunction(const InputParameters & parameters)
  : IntegratedBC(parameters),
    _T_infinity(getFunction("T_infinity")),
    _coefficient(getFunction("coefficient")),
    _coef_func_type(getParam<MooseEnum>("coefficient_function_type").getEnum<CoefFuncType>())
{
}

Real
ConvectiveFluxFunction::computeQpResidual()
{
  Real coef;
  if (_coef_func_type == CoefFuncType::TIME_AND_POSITION)
    coef = _coefficient.value(_t, _q_point[_qp]);
  else
    coef = _coefficient.value(_u[_qp], Point());

  return _test[_i][_qp] * coef * (_u[_qp] - _T_infinity.value(_t, _q_point[_qp]));
}

Real
ConvectiveFluxFunction::computeQpJacobian()
{
  if (_coef_func_type == CoefFuncType::TIME_AND_POSITION)
  {
    Real coef = _coefficient.value(_t, _q_point[_qp]);
    return _test[_i][_qp] * coef * _phi[_j][_qp];
  }
  else
  {
    const Real coef = _coefficient.value(_u[_qp]);
    const Real dcoef_dT = _coefficient.timeDerivative(_u[_qp]);
    return _test[_i][_qp] * (coef + (_u[_qp] - _T_infinity.value(_t, _q_point[_qp])) * dcoef_dT) *
           _phi[_j][_qp];
  }
}
