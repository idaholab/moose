//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ElementVectorL2Error.h"
#include "Function.h"

registerMooseObject("MooseApp", ElementVectorL2Error);

InputParameters
ElementVectorL2Error::validParams()
{
  InputParameters params = ElementIntegralPostprocessor::validParams();
  params.addRequiredParam<FunctionName>("function_x", "The analytic solution to compare against");
  params.addParam<FunctionName>("function_y", 0, "The analytic solution to compare against");
  params.addParam<FunctionName>("function_z", 0, "The analytic solution to compare against");
  params.addRequiredCoupledVar("var_x", "The FE solution in x direction");
  params.addCoupledVar("var_y", 0, "The FE solution in y direction");
  params.addCoupledVar("var_z", 0, "The FE solution in z direction");

  params.addClassDescription("Computes the Vector L2 difference of up to three variables "
                             "simultaneously (normally x, y, z)");
  return params;
}

ElementVectorL2Error::ElementVectorL2Error(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _funcx(getFunction("function_x")),
    _funcy(getFunction("function_y")),
    _funcz(getFunction("function_z")),
    _u(coupledValue("var_x")),
    _v(coupledValue("var_y")),
    _w(coupledValue("var_z"))
{
}

Real
ElementVectorL2Error::getValue()
{
  return std::sqrt(ElementIntegralPostprocessor::getValue());
}

Real
ElementVectorL2Error::computeQpIntegral()
{
  RealVectorValue sol_val(0.0, 0.0, 0.0);
  RealVectorValue func_val(0.0, 0.0, 0.0);

  sol_val(0) = _u[_qp];                          // required variable
  func_val(0) = _funcx.value(_t, _q_point[_qp]); // required function

  sol_val(1) = _v[_qp];
  sol_val(2) = _w[_qp];

  func_val(1) = _funcy.value(_t, _q_point[_qp]);
  func_val(2) = _funcz.value(_t, _q_point[_qp]);

  return (sol_val - func_val).norm_sq(); // dot product of difference vector
}
