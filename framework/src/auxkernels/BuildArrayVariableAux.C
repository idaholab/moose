//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BuildArrayVariableAux.h"

#include <algorithm>

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
  : ArrayAuxKernel(parameters), _component_vars(coupledValues("component_variables"))
{
  // Check the number of component variables
  if (_component_vars.size() != _var.count())
    paramError("variable",
               "The array variable has ",
               _var.count(),
               " components, but ",
               _component_vars.size(),
               " component variables were specified.");

  // List the supported FETypes
  std::vector<FEType> supported_fe_types{{Order::CONSTANT, FEFamily::MONOMIAL},
                                         {Order::FIRST, FEFamily::LAGRANGE}};

  // Check the FEType of the output variable
  if (std::find(supported_fe_types.begin(), supported_fe_types.end(), _var.feType()) ==
      supported_fe_types.end())
    paramError(
        "variable",
        "BuildArrayVariableAux only supports constant monomial or linear Lagrange variables");

  // Make sure the FEType of each input variable matches the output type
  const auto & var_names = parameters.get<std::vector<VariableName>>("component_variables");
  THREAD_ID tid = parameters.get<THREAD_ID>("_tid");
  for (VariableName name : var_names)
  {
    const MooseVariableFieldBase & var_base = _subproblem.getVariable(tid, name);
    if (var_base.feType() != _var.feType())
      paramError(
          "component_variables",
          "The input and output variables of BuildArrayVariableAux must have the same FE type");
  }
}

RealEigenVector
BuildArrayVariableAux::computeValue()
{
  // Get the value of each component
  RealEigenVector out(_component_vars.size());
  for (size_t i = 0; i < _component_vars.size(); ++i)
    out(i) = (*_component_vars[i])[_qp];

  return out;
}
