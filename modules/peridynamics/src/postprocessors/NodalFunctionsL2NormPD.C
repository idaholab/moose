//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalFunctionsL2NormPD.h"
#include "Function.h"

registerMooseObject("PeridynamicsApp", NodalFunctionsL2NormPD);

template <>
InputParameters
validParams<NodalFunctionsL2NormPD>()
{
  InputParameters params = validParams<NodalIntegralPostprocessorBasePD>();
  params.addClassDescription("Class for computing the L2 norm of function(s)");

  params.addParam<FunctionName>("function_0", "The known function 0.");
  params.addParam<FunctionName>("function_1", "The known function 1.");
  params.addParam<FunctionName>("function_2", "The known function 2.");

  return params;
}

NodalFunctionsL2NormPD::NodalFunctionsL2NormPD(const InputParameters & parameters)
  : NodalIntegralPostprocessorBasePD(parameters),
    _has_func_0(isParamValid("function_0")),
    _func_0(_has_func_0 ? &getFunction("function_0") : NULL),
    _has_func_1(isParamValid("function_1")),
    _func_1(_has_func_1 ? &getFunction("function_1") : NULL),
    _has_func_2(isParamValid("function_2")),
    _func_2(_has_func_2 ? &getFunction("function_2") : NULL)
{
  if (_has_func_0 + _has_func_1 + _has_func_2 == 0)
    mooseError("Must provide at least ONE function and up to THREE for L2 norm calculation!");
}

Real
NodalFunctionsL2NormPD::getValue()
{
  return std::sqrt(NodalIntegralPostprocessorBasePD::getValue());
}

Real
NodalFunctionsL2NormPD::computeNodalValue()
{
  Real func_val = 0;

  if (_has_func_0)
    func_val += _func_0->value(_t, *_current_node) * _func_0->value(_t, *_current_node);

  if (_has_func_1)
    func_val += _func_1->value(_t, *_current_node) * _func_1->value(_t, *_current_node);

  if (_has_func_2)
    func_val += _func_2->value(_t, *_current_node) * _func_2->value(_t, *_current_node);

  return func_val;
}
