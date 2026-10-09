//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "KokkosConvectiveFluxFunction.h"

registerKokkosResidualObject("HeatTransferApp", KokkosConvectiveFluxFunction);

InputParameters
KokkosConvectiveFluxFunction::validParams()
{
  InputParameters params = IntegratedBCValue::validParams();
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

KokkosConvectiveFluxFunction::KokkosConvectiveFluxFunction(const InputParameters & parameters)
  : IntegratedBCValue(parameters),
    _T_infinity(getKokkosFunction("T_infinity")),
    _coefficient(getKokkosFunction("coefficient")),
    _coef_of_temperature(getParam<MooseEnum>("coefficient_function_type") == "TEMPERATURE")
{
}
