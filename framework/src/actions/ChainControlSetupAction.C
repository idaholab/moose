//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ChainControlSetupAction.h"
#include "ChainControlDataSystem.h"
#include "ChainControl.h"
#include "FEProblemBase.h"

registerMooseAction("MooseApp", ChainControlSetupAction, "chain_control_setup");

InputParameters
ChainControlSetupAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Performs various setup tasks and checks for ChainControls.");
  return params;
}

ChainControlSetupAction::ChainControlSetupAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
ChainControlSetupAction::act()
{
  // Get the ChainControlData map
  const auto & chain_control_data_map =
      getMooseApp().getChainControlDataSystem().getChainControlDataMap();

  // Check that all chain control data that was retrieved was declared somewhere
  for (const auto & item : chain_control_data_map)
    if (!item.second->getDeclared())
      mooseError("The chain control data '", item.first, "' was requested but never declared.");

  // Get the ChainControls
  auto & control_warehouse = _problem->getControlWarehouse();

  // Call init() on each ChainControl
  for (auto & control_shared_ptr : control_warehouse.getObjects())
  {
    auto chain_control = dynamic_cast<ChainControl *>(control_shared_ptr.get());
    if (chain_control)
      chain_control->init();
  }

  // Copy initial current values back into old values.
  // Note that if an "older" state value is ever added, this will need to be called twice.
  getMooseApp().getChainControlDataSystem().copyValuesBack();

  // Add ChainControl dependencies based on ChainControlData dependencies
  for (auto & control_shared_ptr : control_warehouse.getObjects())
  {
    auto chain_control = dynamic_cast<ChainControl *>(control_shared_ptr.get());
    if (chain_control)
    {
      // Get the control's dependencies on control data
      const auto & data_dep_names = chain_control->getChainControlDataDependencies();
      for (const auto & data_dep_name : data_dep_names)
      {
        // Get the name of the control object that declared the control data
        const auto & data_dep = *chain_control_data_map.at(data_dep_name);
        const auto control_dep_name = data_dep.getChainControl().name();

        // Add this name to the list of the control's dependencies if not present
        auto & control_deps = chain_control->getDependencies();
        if (std::find(control_deps.begin(), control_deps.end(), control_dep_name) ==
            control_deps.end())
          control_deps.push_back(control_dep_name);
      }
    }
  }
}
