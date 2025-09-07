//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ReactionNetworkPhysicsBase.h"
#include <sstream>

InputParameters
ReactionNetworkPhysicsBase::validParams()
{
  InputParameters params = PhysicsBase::validParams();
  params.addClassDescription("Base class to create a reaction network Physics from.");

  // Variables parameters
  params.addRequiredParam<std::vector<VariableName>>("solver_variables", "Species to solve for");
  params.addRequiredParam<std::vector<AuxVariableName>>(
      "auxiliary_variables",
      "Additional species to output the concentration of, which do not require an additional "
      "equation to obtain");
  params.transferParam<MooseEnum>(MooseVariableBase::validParams(), "order", "variable_order");
  params.addParam<std::vector<Real>>("equation_scaling",
                                     "Scaling factor to apply to each equation");

  // Initial conditions
  params.addParam<std::vector<MooseFunctorName>>("initial_conditions",
                                                 "Initial conditions for the species to solve for");

  // Reaction network
  params.addParam<std::string>("reactions", "The list of chemical reactions");

  return params;
}

ReactionNetworkPhysicsBase::ReactionNetworkPhysicsBase(const InputParameters & parameters)
  : PhysicsBase(parameters),
    _solver_species(getParam<std::vector<VariableName>>("solver_variables")),
    _num_solver_species(_solver_species.size()),
    _aux_species(getParam<std::vector<AuxVariableName>>("auxiliary_variables")),
    _num_aux_species(_aux_species.size()),
    _reactions(
        ReactionNetworkUtils::parseReactionNetwork(getParam<std::string>("reactions"), _verbose)),
    _num_reactions(_reactions.size())
{
  // Keep track of variables
  for (const auto & var_name : _solver_species)
    saveSolverVariableName(var_name);
  for (const auto & var_name : _aux_species)
    saveAuxVariableName(var_name);

  // Parameter checking
  if (isParamSetByUser("additional_source_coefs"))
    checkParamsBothSetOrNotSet("source_functors", "additional_source_coefs");
  if (isParamValid("additional_source_functors"))
    checkVectorParamsSameLength<VariableName, MooseFunctorName>("solver_variables",
                                                                "additional_source_functors");
  if (isParamValid("initial_conditions"))
    checkVectorParamsSameLength<VariableName, FunctionName>("solver_variables",
                                                            "initial_conditions");

  addRequiredPhysicsTask("add_ic");
  addRequiredPhysicsTask("add_variable");
  addRequiredPhysicsTask("add_aux_variable");
  addRequiredPhysicsTask("init_physics");

  // Parse the lines in the reaction
  std::stringstream reactions_param(getParam<std::string>("reactions"));
  std::string line;
  while (std::getline(reactions_param, line, '\n'))
    _reactions_input.push_back(line);
  mooseAssert(_num_reactions == _reactions_input.size(),
              "Should be the same size. Extra line break in the reaction network?");
}

void
ReactionNetworkPhysicsBase::addSolverVariables()
{
  for (const auto i : index_range(_solver_species))
  {
    const auto & var_name = _solver_species[i];
    // If the variable was added outside the Physics
    if (variableExists(var_name, /*error_if_aux*/ true))
    {
      reportPotentiallyMissedParameters(
          {"variable_order", "system_names", "equation_scaling"}, "MooseVariable", var_name);
      continue;
    }

    const std::string variable_type = "MooseVariable";
    InputParameters params = getFactory().getValidParams(variable_type);
    params.set<MooseEnum>("order") = getParam<MooseEnum>("variable_order");
    params.set<Real>("scaling") = getParam<std::vector<Real>>("equation_scaling")[i];
    assignBlocks(params, _blocks);
    params.set<SolverSystemName>("solver_sys") = getSolverSystem(var_name);

    getProblem().addVariable(variable_type, var_name, params);
  }
}

void
ReactionNetworkPhysicsBase::addAuxiliaryVariables()
{
  for (const auto i : index_range(_aux_species))
  {
    const auto & var_name = _aux_species[i];
    // If the variable was added outside the Physics
    if (variableExists(var_name, /*error_if_aux*/ false))
    {
      reportPotentiallyMissedParameters({"variable_order"}, "MooseVariable", var_name);
      continue;
    }

    const std::string variable_type = "MooseVariable";
    InputParameters params = getFactory().getValidParams(variable_type);
    params.set<MooseEnum>("order") = getParam<MooseEnum>("variable_order");
    assignBlocks(params, _blocks);
    getProblem().addAuxVariable(variable_type, var_name, params);
  }
}

void
ReactionNetworkPhysicsBase::addPreconditioning()
{
  // TODO: identify fully independent or sequential groups in the reaction network
  // Solve using a segregated approach in the order of the sequence
}

void
ReactionNetworkPhysicsBase::addComponent(const ActionComponent & component)
{
  for (const auto & block : component.blocks())
    _blocks.push_back(block);
}

void
ReactionNetworkPhysicsBase::addInitialConditions()
{
  InputParameters params = getFactory().getValidParams("FunctorIC");
  assignBlocks(params, _blocks);
  std::vector<MooseFunctorName> empty_functor_vector;
  const auto & initial_functors =
      isParamValid("initial_conditions")
          ? getParam<std::vector<MooseFunctorName>>("initial_conditions")
          : empty_functor_vector;

  for (const auto i : index_range(_solver_species))
  {
    const auto & var_name = _solver_species[i];
    // always obey the user specification of initial conditions
    // Base class does not have a default but derived classes could set one
    if (isParamValid("initial_conditions") &&
        shouldCreateIC(var_name,
                       _blocks,
                       /*whether IC is a default*/ !isParamSetByUser("initial_conditions"),
                       /*error if already an IC*/ true))
    {
      params.set<VariableName>("variable") = var_name;
      params.set<MooseFunctorName>("functor") = initial_functors[i];
      getProblem().addInitialCondition("FunctorIC", prefix() + var_name + "_ic", params);
    }
  }
}
