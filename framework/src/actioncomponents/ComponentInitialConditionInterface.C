//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComponentInitialConditionInterface.h"

InputParameters
ComponentInitialConditionInterface::validParams()
{
  auto params = ActionComponent::validParams();
  params.addParam<std::vector<VariableName>>(
      "initial_condition_variables",
      {},
      "List of variables that should have an initial "
      "condition defined on the blocks of this ActionComponent");
  params.addParam<std::vector<MooseFunctorName>>(
      "initial_condition_values",
      {},
      "Functors that provide the initial values of the variables on this ActionComponent");

  params.addParamNamesToGroup("initial_condition_variables initial_condition_values",
                              "Variable initialization");
  return params;
}

ComponentInitialConditionInterface::ComponentInitialConditionInterface(
    const InputParameters & params)
  : ActionComponent(params),
    _initial_condition_variables(
        getParam<std::vector<VariableName>>("initial_condition_variables")),
    _variable_ic_functors(getParam<std::vector<MooseFunctorName>>("initial_condition_values"))
{
  addRequiredTask("init_component_physics");
  addRequiredTask("check_integrity");

  // Parameter checks
  checkVectorParamsNoOverlap<VariableName>({"initial_condition_variables"});
  if (_initial_condition_variables.size() != _variable_ic_functors.size())
    paramError("initial_condition_variables",
               "Should be the same size as 'initial_condition_values'");
}

bool
ComponentInitialConditionInterface::hasInitialCondition(const VariableName & var_name) const
{
  return std::find(_initial_condition_variables.begin(),
                   _initial_condition_variables.end(),
                   var_name) != _initial_condition_variables.end();
}

const MooseFunctorName &
ComponentInitialConditionInterface::getInitialCondition(const VariableName & variable,
                                                        const std::string & requestor_name) const
{
  // Ideally all initial conditions defined by the user in the input will get requested
  _requested_ic_variables.insert(variable);

  // Sorted by the user in the input parameters
  for (const auto i : index_range(_initial_condition_variables))
    if (_initial_condition_variables[i] == variable)
      return _variable_ic_functors[i];
  paramError("initial_condition_variables",
             "Initial condition for variable '" + variable + "' requested by '" + requestor_name +
                 "' has not been specified on this ActionComponent.");
}

void
ComponentInitialConditionInterface::checkInitialConditionsAllRequested() const
{
  if (_requested_ic_variables.size() != _initial_condition_variables.size())
  {
    std::string list_missing = "";
    for (const auto & ic_name : _initial_condition_variables)
      if (_requested_ic_variables.count(ic_name) == 0)
        list_missing = (list_missing == "" ? "" : ", ") + ic_name;

    paramError("initial_condition_variables",
               "Initial conditions for variables '" + list_missing +
                   "' have been defined on this ActionComponent, but have not been requested by "
                   "any Physics.");
  }
}
