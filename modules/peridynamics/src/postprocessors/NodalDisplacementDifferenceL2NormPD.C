//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalDisplacementDifferenceL2NormPD.h"
#include "MooseVariable.h"
#include "Function.h"

registerMooseObject("PeridynamicsApp", NodalDisplacementDifferenceL2NormPD);

template <>
InputParameters
validParams<NodalDisplacementDifferenceL2NormPD>()
{
  InputParameters params = validParams<NodalIntegralPostprocessorBasePD>();
  params.addClassDescription("Class for computing the L2 norm of the difference between "
                             "displacement(s) and it/their analytical solution(s)");

  params.addParam<FunctionName>("function_0", "The known function for displacement component 0.");
  params.addParam<FunctionName>("function_1", "The known function for displacement component 1.");
  params.addParam<FunctionName>("function_2", "The known function for displacement component 2.");
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "displacements", "Nonlinear variable name for the displacements");

  return params;
}

NodalDisplacementDifferenceL2NormPD::NodalDisplacementDifferenceL2NormPD(
    const InputParameters & parameters)
  : NodalIntegralPostprocessorBasePD(parameters),
    _has_func_0(isParamValid("function_0")),
    _func_0(_has_func_0 ? &getFunction("function_0") : NULL),
    _has_func_1(isParamValid("function_1")),
    _func_1(_has_func_1 ? &getFunction("function_1") : NULL),
    _has_func_2(isParamValid("function_2")),
    _func_2(_has_func_2 ? &getFunction("function_2") : NULL)
{
  if (_has_func_0 + _has_func_1 + _has_func_2 == 0)
    mooseError("Must provide at least one displacement function for integral error check!");

  const std::vector<NonlinearVariableName> & nl_vnames(
      getParam<std::vector<NonlinearVariableName>>("displacements"));

  if (nl_vnames.size() > _dim)
    mooseError("Number of displacements component should not greater than problem dimension!");

  for (unsigned int i = 0; i < nl_vnames.size(); ++i)
    _disp_var.push_back(&_subproblem.getStandardVariable(_tid, nl_vnames[i]));
}

Real
NodalDisplacementDifferenceL2NormPD::getValue()
{
  return std::sqrt(NodalIntegralPostprocessorBasePD::getValue());
}

Real
NodalDisplacementDifferenceL2NormPD::computeNodalValue()
{
  Real diff = 0;
  if (_has_func_0)
  {
    diff += (_disp_var[0]->getNodalValue(*_current_node) - _func_0->value(_t, *_current_node)) *
            (_disp_var[0]->getNodalValue(*_current_node) - _func_0->value(_t, *_current_node));
  }
  if (_has_func_1)
  {
    diff += (_disp_var[1]->getNodalValue(*_current_node) - _func_1->value(_t, *_current_node)) *
            (_disp_var[1]->getNodalValue(*_current_node) - _func_1->value(_t, *_current_node));
  }
  if (_has_func_2)
  {
    diff += (_disp_var[2]->getNodalValue(*_current_node) - _func_2->value(_t, *_current_node)) *
            (_disp_var[2]->getNodalValue(*_current_node) - _func_2->value(_t, *_current_node));
  }

  return diff;
}
