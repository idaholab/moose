//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsVariableCoordinator.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "FEProblemBase.h"

registerMooseAction("MooseApp", PhysicsVariableCoordinator, "flush_shared_physics_variables");

InputParameters
PhysicsVariableCoordinator::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Merges the requests of multiple Physics for the same shared variable into a single "
      "variable spanning the union of their blocks. Created on demand; not meant to be declared "
      "in an input file.");
  return params;
}

PhysicsVariableCoordinator::PhysicsVariableCoordinator(const InputParameters & params)
  : Action(params)
{
}

void
PhysicsVariableCoordinator::requestVariable(const VariableName & var_name,
                                            const VariableRequest & request,
                                            const std::string & requester)
{
  const auto it = _requests.find(var_name);
  if (it == _requests.end())
  {
    _requests.emplace(var_name, MergedRequest{request, requester});
    return;
  }

  auto & merged = it->second;
  const auto & existing = merged._request;
  if (existing._nl != request._nl || existing._family != request._family ||
      existing._order != request._order || existing._scaling != request._scaling ||
      existing._solver_sys != request._solver_sys)
    mooseError("Physics '",
               requester,
               "' requests the shared variable '",
               var_name,
               "' with a different variable type, family, order, scaling factor or solver system "
               "than Physics '",
               merged._requester,
               "' did.");

  auto & blocks = merged._request._blocks;
  for (const auto & block : request._blocks)
    if (std::find(blocks.begin(), blocks.end(), block) == blocks.end())
      blocks.push_back(block);
}

void
PhysicsVariableCoordinator::act()
{
  for (const auto & [var_name, merged] : _requests)
  {
    const auto & req = merged._request;
    const std::string class_name = "MooseVariable";
    InputParameters params = _factory.getValidParams(class_name);
    // ANY_BLOCK_ID cannot be passed to addVariable(); leaving 'block' unset has the same meaning
    if (std::find(req._blocks.begin(), req._blocks.end(), "ANY_BLOCK_ID") == req._blocks.end())
      params.set<std::vector<SubdomainName>>("block") = req._blocks;
    params.set<MooseEnum>("family") = req._family;
    params.set<MooseEnum>("order") = req._order;
    if (req._nl)
    {
      params.set<SolverSystemName>("solver_sys") = req._solver_sys;
      params.set<std::vector<Real>>("scaling") = {req._scaling};
      _problem->addVariable(class_name, var_name, params);
    }
    else
      _problem->addAuxVariable(class_name, var_name, params);
  }
}

PhysicsVariableCoordinator &
PhysicsVariableCoordinator::findOrCreate(ActionWarehouse & awh, ActionFactory & action_factory)
{
  const auto existing = awh.getActions<PhysicsVariableCoordinator>();
  if (!existing.empty())
    return *const_cast<PhysicsVariableCoordinator *>(existing[0]);

  const std::string class_name = "PhysicsVariableCoordinator";
  InputParameters params = action_factory.getValidParams(class_name);
  params.set<bool>("_built_by_moose") = true;
  params.set<std::string>("registered_identifier") = "(AutoBuilt)";

  auto action = action_factory.create(class_name, "physics_variable_coordinator", params);
  auto * coordinator = dynamic_cast<PhysicsVariableCoordinator *>(action.get());
  if (!coordinator)
    ::mooseError("Internal error: failed to create the PhysicsVariableCoordinator");
  awh.addActionBlock(action);
  return *coordinator;
}
