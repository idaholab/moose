//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalVariableIntegralPD.h"
#include "MooseVariable.h"

registerMooseObject("PeridynamicsApp", NodalVariableIntegralPD);

InputParameters
NodalVariableIntegralPD::validParams()
{
  InputParameters params = NodalIntegralPostprocessorBasePD::validParams();
  params.addClassDescription("Class for calculating the domain integral of nodal variables");

  params.addRequiredParam<VariableName>(
      "variable", "The name of the variable that this postprocessor operates on");

  return params;
}

NodalVariableIntegralPD::NodalVariableIntegralPD(const InputParameters & parameters)
  : NodalIntegralPostprocessorBasePD(parameters),
    _var(_subproblem.getStandardVariable(_tid, getParam<VariableName>("variable")))
{
}

Real
NodalVariableIntegralPD::computeNodalValue()
{
  return _var.getNodalValue(*_current_node);
}
