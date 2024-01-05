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
  params.addClassDescription("Returns the L2-norm of the difference between a pair of computed "
                             "and analytical vector-valued solutions.");
  params.addParam<FunctionName>("function", 0, "The vector analytical solution to compare against");
  params.addParam<FunctionName>(
      "function_x", 0, "The analytical solution to compare against in the x direction");
  params.addParam<FunctionName>(
      "function_y", 0, "The analytical solution to compare against in the y direction");
  params.addParam<FunctionName>(
      "function_z", 0, "The analytical solution to compare against in the z direction");
  params.addCoupledVar("variable", {0, 0, 0}, "The vector FE solution");
  params.addCoupledVar("var_x", 0, "The FE solution in the x direction");
  params.addCoupledVar("var_y", 0, "The FE solution in the y direction");
  params.addCoupledVar("var_z", 0, "The FE solution in the z direction");
  return params;
}

ElementVectorL2Error::ElementVectorL2Error(const InputParameters & parameters)
  : ElementIntegralPostprocessor(parameters),
    _func(getFunction("function")),
    _funcx(getFunction("function_x")),
    _funcy(getFunction("function_y")),
    _funcz(getFunction("function_z")),
    _u(coupledVectorValue("variable")),
    _ux(coupledValue("var_x")),
    _uy(coupledValue("var_y")),
    _uz(coupledValue("var_z")),
    _has_vector_function(isParamSetByUser("function")),
    _has_scalar_function(isParamSetByUser("function_x") || isParamSetByUser("function_y") ||
                         isParamSetByUser("function_z")),
    _has_vector_variable(isParamSetByUser("variable")),
    _has_scalar_variable(isParamSetByUser("var_x") || isParamSetByUser("var_y") ||
                         isParamSetByUser("var_z"))
{
  if (!_has_vector_function && !_has_scalar_function)
    paramError("function",
               "The 'function' and 'function_{x,y,z}' parameters cannot both be unset.");

  if (_has_vector_function && _has_scalar_function)
    paramError("function", "The 'function' and 'function_{x,y,z}' parameters cannot both be set.");

  if (!_has_vector_variable && !_has_scalar_variable)
    paramError("variable", "The 'variable' and 'var_{x,y,z}' parameters cannot both be unset.");

  if (_has_vector_variable && _has_scalar_variable)
    paramError("variable", "The 'variable' and 'var_{x,y,z}' parameters cannot both be set.");
}

Real
ElementVectorL2Error::getValue() const
{
  return std::sqrt(ElementIntegralPostprocessor::getValue());
}

Real
ElementVectorL2Error::computeQpIntegral()
{
  RealVectorValue func_val = _has_vector_function
                                 ? _func.vectorValue(_t, _q_point[_qp])
                                 : RealVectorValue(_funcx.value(_t, _q_point[_qp]),
                                                   _funcy.value(_t, _q_point[_qp]),
                                                   _funcz.value(_t, _q_point[_qp]));

  RealVectorValue sol_val =
      _has_vector_variable ? _u[_qp] : RealVectorValue(_ux[_qp], _uy[_qp], _uz[_qp]);

  return (sol_val - func_val).norm_sq(); // dot product of difference vector
}
