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

InputParameters
NodalDisplacementDifferenceL2NormPD::validParams()
{
  InputParameters params = NodalIntegralPostprocessorBasePD::validParams();
  params.addClassDescription("Class for computing the L2 norm of the difference between "
                             "displacements and their analytic solutions");

  params.addRequiredParam<std::vector<FunctionName>>(
      "analytic_functions", "The known analytic functions for displacements");
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "displacements", "Nonlinear variable name for the displacements");

  return params;
}

NodalDisplacementDifferenceL2NormPD::NodalDisplacementDifferenceL2NormPD(
    const InputParameters & parameters)
  : NodalIntegralPostprocessorBasePD(parameters)
{
  const std::vector<NonlinearVariableName> & nl_vnames(
      getParam<std::vector<NonlinearVariableName>>("displacements"));

  const std::vector<FunctionName> & func_names(
      getParam<std::vector<FunctionName>>("analytic_functions"));

  _n_disps = nl_vnames.size();
  if (_n_disps > _dim)
    mooseError("Number of displacements components should not greater than problem dimension!");

  if (_n_disps != func_names.size())
    mooseError("Number of analytic_functions components should be the same as the number of "
               "displacements components!");

  for (unsigned int i = 0; i < _n_disps; ++i)
  {
    _disp_var.push_back(&_subproblem.getStandardVariable(_tid, nl_vnames[i]));
    _funcs.push_back(&getFunctionByName(func_names[i]));
  }
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
  for (unsigned int i = 0; i < _n_disps; ++i)
    diff += (_disp_var[i]->getNodalValue(*_current_node) - _funcs[i]->value(_t, *_current_node)) *
            (_disp_var[i]->getNodalValue(*_current_node) - _funcs[i]->value(_t, *_current_node));

  return diff;
}
