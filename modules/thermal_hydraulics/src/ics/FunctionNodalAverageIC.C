//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FunctionNodalAverageIC.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", FunctionNodalAverageIC);

InputParameters
FunctionNodalAverageIC::validParams()
{
  InputParameters params = InitialCondition::validParams();

  params.addClassDescription(
      "Initial conditions for an elemental variable from a function using nodal average.");

  params.addRequiredParam<FunctionName>("function", "The initial condition function.");

  return params;
}

FunctionNodalAverageIC::FunctionNodalAverageIC(const InputParameters & parameters)
  : InitialCondition(parameters), _func(getFunction("function"))
{
}

Real
FunctionNodalAverageIC::value(const Point & /*p*/)
{
  const unsigned int n_nodes = _current_elem->n_nodes();

  Real sum = 0.0;
  for (unsigned int i = 0; i < n_nodes; i++)
  {
    const Node & node = _current_elem->node_ref(i);
    sum += _func.value(_t, node);
  }

  return sum / n_nodes;
}
