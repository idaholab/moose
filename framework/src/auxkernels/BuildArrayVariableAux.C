//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BuildArrayVariableAux.h"

registerMooseObject("MooseApp", BuildArrayVariableAux);

InputParameters
BuildArrayVariableAux::validParams()
{
  InputParameters params = ArrayAuxKernel::validParams();
  params.addCoupledVar("component_variables",
                       "The variables that make up each component of the output array variable.");
  params.addClassDescription("Combines multiple standard variables into an array variable.");
  return params;
}

BuildArrayVariableAux::BuildArrayVariableAux(const InputParameters & parameters)
  : ArrayAuxKernel(parameters), _component_dofs(coupledAllDofValues("component_variables"))
{
  // Check the number of component variables
  if (_component_dofs.size() != _var.count())
    paramError("variable",
               "The array variable has ",
               _var.count(),
               " components, but ",
               _component_dofs.size(),
               " component variables were specified.");

  // Make sure the FEType of each input variable matches the output type
  for (const auto & var : getCoupledMooseVars())
    if (var->feType() != _var.feType())
      paramError("component_variables",
                 "The input and output variables must have the same FE type");
}

void
BuildArrayVariableAux::compute()
{
  const auto n_local_dofs = _var.numberOfDofs();
  _local_sol.resize(n_local_dofs);
  for (MooseIndex(n_local_dofs) j = 0; j < n_local_dofs; ++j)
  {
    _local_sol(j).resize(_var.count());
    for (MooseIndex(_var.count()) i = 0; i < _var.count(); ++i)
      _local_sol(j)(i) = (*_component_dofs[i])[j];
  }
  _var.setDofValues(_local_sol);
}
