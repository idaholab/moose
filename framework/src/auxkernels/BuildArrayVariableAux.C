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

defineLegacyParams(BuildArrayVariableAux);

InputParameters
BuildArrayVariableAux::validParams()
{
  InputParameters params = ArrayAuxKernel::validParams();
  params.addParam<std::vector<VariableName>>("component_variables",
    "The variables that make up each component of the output array variable.");
  params.addClassDescription(
    "Copy multiple variables into the components of an array variable.");
  return params;
}

BuildArrayVariableAux::BuildArrayVariableAux(const InputParameters & parameters)
  : ArrayAuxKernel(parameters)
{
  const auto & var_names
    = parameters.get<std::vector<VariableName>>("component_variables");

  // Check the number of component variables
  if (var_names.size() != _var.count())
    mooseError("The array variable has ", _var.count(),
               " components, but ", var_names.size(),
               " component variables were specified.");

  // Get pointers to each of the component vars
  THREAD_ID tid = parameters.get<THREAD_ID>("_tid");
  for (VariableName name : var_names)
    _component_vars.push_back(& _subproblem.getVariable(tid, name));

  // Check the FEType of the output variable
  if ((_var.feType().family != FEFamily::MONOMIAL)
      || (_var.feType().order != Order::CONSTANT))
    mooseError(
      "BuildArrayVariableAux only supports constant monomial variables");

  // Check the FEType of the component vars
  for (auto var : _component_vars) {
    if ((var->feType().family != FEFamily::MONOMIAL)
        || (var->feType().order != Order::CONSTANT))
      mooseError(
        "BuildArrayVariableAux only supports constant monomial variables");
   }
}

RealEigenVector
BuildArrayVariableAux::computeValue()
{
  // This function assumes that DoFs are indexed by element and that there is
  // only 1 DoF per element, i.e. that all variables are constant monomials.
  constexpr unsigned int fe_comp {0};

  // Get the value of each component
  std::vector<Real> component_vals;
  component_vals.reserve(_component_vars.size());
  for (auto var : _component_vars) {
    unsigned int sys_num = var->sys().number();
    unsigned int var_num = var->number();
    dof_id_type component_dof = _current_elem->dof_number(sys_num, var_num,
                                                          fe_comp);
    component_vals.push_back(var->sys().solution()(component_dof));
  }

  // Convert the output to an Eigen matrix and return
  RealEigenVector out;
  out = Eigen::Map<RealEigenVector>(component_vals.data(),
                                    component_vals.size());
  return out;
}
