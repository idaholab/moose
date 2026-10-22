//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "THMVariableCoordinator.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "FEProblemBase.h"

registerMooseAction("ThermalHydraulicsApp", THMVariableCoordinator, "THM:flush_shared_variables");

InputParameters
THMVariableCoordinator::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Merges multiple components' requests for the same shared, bare-named variable (e.g. "
      "'rhoA', 'T_solid') into a single MOOSE variable spanning the union of their blocks. "
      "Auto-created; not meant to be declared in an input file.");
  return params;
}

THMVariableCoordinator::THMVariableCoordinator(const InputParameters & params) : Action(params) {}

void
THMVariableCoordinator::requestVariable(bool nl,
                                        const VariableName & var_name,
                                        const std::string & family,
                                        const std::string & order,
                                        Real scaling,
                                        const std::vector<SubdomainName> & blocks)
{
  const auto it = _requests.find(var_name);
  if (it == _requests.end())
    _requests.emplace(var_name, VariableRequest{nl, family, order, scaling, blocks});
  else
    for (const auto & block : blocks)
      if (std::find(it->second._blocks.begin(), it->second._blocks.end(), block) ==
          it->second._blocks.end())
        it->second._blocks.push_back(block);
}

void
THMVariableCoordinator::act()
{
  if (_current_task != "THM:flush_shared_variables")
    return;

  for (const auto & [var_name, req] : _requests)
  {
    const std::string class_name = "MooseVariable";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<std::vector<SubdomainName>>("block") = req._blocks;
    params.set<MooseEnum>("family") = req._family;
    params.set<MooseEnum>("order") = req._order;
    if (req._nl)
    {
      params.set<std::vector<Real>>("scaling") = {req._scaling};
      _problem->addVariable(class_name, var_name, params);
    }
    else
      _problem->addAuxVariable(class_name, var_name, params);
  }
}

THMVariableCoordinator &
THMVariableCoordinator::findOrCreate(ActionWarehouse & awh, ActionFactory & action_factory)
{
  const auto existing = awh.getActions<THMVariableCoordinator>();
  if (!existing.empty())
    return *const_cast<THMVariableCoordinator *>(existing[0]);

  const std::string class_name = "THMVariableCoordinator";
  InputParameters params = action_factory.getValidParams(class_name);
  params.set<bool>("_built_by_moose") = true;
  params.set<std::string>("registered_identifier") = "(AutoBuilt)";

  auto action = action_factory.create(class_name, "thm_variable_coordinator", params);
  auto * coordinator = dynamic_cast<THMVariableCoordinator *>(action.get());
  if (!coordinator)
    ::mooseError("Internal error: failed to create the THMVariableCoordinator");
  awh.addActionBlock(action);
  return *coordinator;
}
