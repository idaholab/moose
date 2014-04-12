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

template<>
InputParameters validParams<ElementVectorL2Error>()
{
  InputParameters params = validParams<ElementIntegralPostprocessor>();
  params.addRequiredParam<FunctionName>("function_x", "The analytic solution to compare against");
  params.addParam<FunctionName>("function_y", "The analytic solution to compare against");
  params.addParam<FunctionName>("function_z", "The analytic solution to compare against");
  params.addRequiredCoupledVar("var_x","The FE solution in x direction");
  params.addCoupledVar("var_y","The FE solution in y direction");
  params.addCoupledVar("var_z","The FE solution in z direction");
  return params;
}

ElementVectorL2Error::ElementVectorL2Error(const std::string & name, InputParameters parameters) :
    ElementIntegralPostprocessor(name, parameters),
    FunctionInterface(parameters),
    _funcx(&getFunction("function_x")),
    _funcy(parameters.isParamValid("function_y") ? &getFunction("function_y") : NULL),
    _funcz(parameters.isParamValid("function_z") ? &getFunction("function_z") : NULL),
    _u(&coupledValue("var_x")),
    _v(isCoupled("var_y") ? &coupledValue("var_y") : NULL),
    _w(isCoupled("var_z") ? &coupledValue("var_z") : NULL)
{
  int num_func = 1; // input function counter
  int num_var = 1;  // input variable counter

  if (_v) num_var += 1;
  if (_w) num_var += 1;

  if (_funcy) num_func += 1;
  if (_funcz) num_func += 1;

  if (num_func != num_var)
    mooseError("Number of input functions and number of input variables are not equal.");
}

Real
ElementVectorL2Error::getValue()
{
  return std::sqrt(ElementIntegralPostprocessor::getValue());
}

Real
ElementVectorL2Error::computeQpIntegral()
{
  RealVectorValue sol_val(0.0,0.0,0.0);
  RealVectorValue func_val(0.0,0.0,0.0);

  sol_val(0) = (*_u)[_qp]; // required variable
  func_val(0) = _funcx->value(_t, _q_point[_qp]); // required function

  if (_v) sol_val(1) = (*_v)[_qp];
  if (_w) sol_val(2) = (*_w)[_qp];

  if (_funcy) func_val(1) = _funcy->value(_t, _q_point[_qp]);
  if (_funcz) func_val(2) = _funcz->value(_t, _q_point[_qp]);

  return (sol_val - func_val).size_sq(); // dot product of difference vector
}
