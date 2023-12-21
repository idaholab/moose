//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementHCurlSemiError.h"
#include "Function.h"

registerMooseObject("MooseApp", ElementHCurlSemiError);

InputParameters
ElementHCurlSemiError::validParams()
{
  InputParameters params = ElementVectorL2Error::validParams();
  params.addClassDescription("Returns the H(curl)-seminorm of the difference between a pair of "
                             "computed and analytical vector-valued solutions.");
  params.addRequiredParam<FunctionName>("function",
                                        "The vector analytical solution to compare against");
  params.addRequiredCoupledVar("variable", "The vector FE solution");
  params.suppressParameter<FunctionName>("function_x");
  params.suppressParameter<FunctionName>("function_y");
  params.suppressParameter<FunctionName>("function_z");
  params.suppressParameter<std::vector<VariableName>>("var_x");
  params.suppressParameter<std::vector<VariableName>>("var_y");
  params.suppressParameter<std::vector<VariableName>>("var_z");
  return params;
}

ElementHCurlSemiError::ElementHCurlSemiError(const InputParameters & parameters)
  : ElementVectorL2Error(parameters),
    _u_var(*getVectorVar("variable", 0)),
    _curl_u(_u_var.curlSln())
{
}

Real
ElementHCurlSemiError::computeQpIntegral()
{
  return (_curl_u[_qp] - _func.curl(_t, _q_point[_qp])).norm_sq();
}
