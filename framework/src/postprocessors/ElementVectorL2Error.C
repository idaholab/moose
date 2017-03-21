/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "ElementVectorL2Error.h"
#include "Function.h"

template <>
InputParameters
validParams<ElementVectorL2Error>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addRequiredParam<FunctionName>("function_x", "The analytic solution to compare against");
  params.addParam<FunctionName>("function_y", 0, "The analytic solution to compare against");
  params.addParam<FunctionName>("function_z", 0, "The analytic solution to compare against");
  params.addRequiredCoupledVar("var_x", "The FE solution in x direction");
  params.addCoupledVar("var_y", 0, "The FE solution in y direction");
  params.addCoupledVar("var_z", 0, "The FE solution in z direction");
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
