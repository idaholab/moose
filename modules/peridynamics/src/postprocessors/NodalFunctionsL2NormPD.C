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

InputParameters
NodalFunctionsL2NormPD::validParams()
{
  InputParameters params = NodalIntegralPostprocessorBasePD::validParams();
  params.addClassDescription("Class for computing the L2 norm of functions");

  params.addRequiredParam<std::vector<FunctionName>>("functions", "The known functions");

  return params;
}

NodalFunctionsL2NormPD::NodalFunctionsL2NormPD(const InputParameters & parameters)
  : NodalIntegralPostprocessorBasePD(parameters)
{
  const std::vector<FunctionName> & func_names(getParam<std::vector<FunctionName>>("functions"));

  _n_funcs = func_names.size();

  for (unsigned int i = 0; i < _n_funcs; ++i)
    _funcs.push_back(&getFunctionByName(func_names[i]));
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

  for (unsigned int i = 0; i < _n_funcs; ++i)
    func_val += _funcs[i]->value(_t, *_current_node) * _funcs[i]->value(_t, *_current_node);

  return func_val;
}
