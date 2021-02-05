//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BuildArrayVariableAux.h"

#include "SystemBase.h"

registerMooseObject("MooseApp", BuildArrayVariableAux);

InputParameters
BuildArrayVariableAux::validParams()
{
  InputParameters params = ArrayAuxKernel::validParams();
  params.addParam<std::vector<VariableName>>(
      "component_variables",
      "The variables that make up each component of the output array variable.");
  params.addClassDescription("Copy multiple variables into the components of an array variable.");
  return params;
}

BuildArrayVariableAux::BuildArrayVariableAux(const InputParameters & parameters)
  : ArrayAuxKernel(parameters)
{
  const auto & var_names = parameters.get<std::vector<VariableName>>("component_variables");

  // Check the number of component variables
  if (var_names.size() != _var.count())
    mooseError("The array variable has ",
               _var.count(),
               " components, but ",
               var_names.size(),
               " component variables were specified.");

  // Get pointers to each of the component VariableValues
  THREAD_ID tid = parameters.get<THREAD_ID>("_tid");
  for (VariableName name : var_names)
  {
    const MooseVariableFieldBase & var_base = _subproblem.getVariable(tid, name);

    // Check the FEType of the component var
    if ((var_base.feType().family != FEFamily::MONOMIAL) ||
        (var_base.feType().order != Order::CONSTANT))
      mooseError("BuildArrayVariableAux only supports constant monomial variables");

    // Try casting to the appropriate type and store the VariableValue
    if (const MooseVariableField<Real> * var =
            dynamic_cast<const MooseVariableField<Real> *>(&var_base))
      _component_vars.push_back(&var->sln());
    else
      mooseError("BuildArrayVariableAux only supports variables of type MooseVariableField<Real>");
  }

  // Check the FEType of the output variable
  if ((_var.feType().family != FEFamily::MONOMIAL) || (_var.feType().order != Order::CONSTANT))
    mooseError("BuildArrayVariableAux only supports constant monomial variables");
}

RealEigenVector
BuildArrayVariableAux::computeValue()
{
  // Get the value of each component
  std::vector<Real> component_vals;
  component_vals.reserve(_component_vars.size());
  for (auto var : _component_vars)
    component_vals.push_back((*var)[_qp]);

  // Convert the output to an Eigen matrix and return
  RealEigenVector out;
  out = Eigen::Map<RealEigenVector>(component_vals.data(), component_vals.size());
  return out;
}
