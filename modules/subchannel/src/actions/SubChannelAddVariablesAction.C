//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubChannelAddVariablesAction.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "AddVariableAction.h"
#include "AddAuxVariableAction.h"
#include "SubChannelApp.h"

registerMooseAction("SubChannelApp", SubChannelAddVariablesAction, "meta_action");

InputParameters
SubChannelAddVariablesAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Adds the variables associated with the subchannel problem");
  return params;
}

SubChannelAddVariablesAction::SubChannelAddVariablesAction(const InputParameters & parameters)
  : Action(parameters),
    _fe_family(AddVariableAction::getNonlinearVariableFamilies()),
    _fe_order(AddVariableAction::getNonlinearVariableOrders())
// Set the block parameters to the hardcoded 'subchannel' and 'pins' domains
{
}

void
SubChannelAddVariablesAction::act()
{
  std::vector<std::string> var_names = {SubChannelApp::MASS_FLOW_RATE,
                                        SubChannelApp::SURFACE_AREA,
                                        SubChannelApp::SUM_CROSSFLOW,
                                        SubChannelApp::PRESSURE,
                                        SubChannelApp::PRESSURE_DROP,
                                        SubChannelApp::WETTED_PERIMETER,
                                        SubChannelApp::LINEAR_HEAT_RATE,
                                        SubChannelApp::DUCT_LINEAR_HEAT_RATE,
                                        SubChannelApp::ENTHALPY,
                                        SubChannelApp::TEMPERATURE,
                                        SubChannelApp::PIN_TEMPERATURE,
                                        SubChannelApp::PIN_DIAMETER,
                                        SubChannelApp::DUCT_TEMPERATURE,
                                        SubChannelApp::DENSITY,
                                        SubChannelApp::VISCOSITY,
                                        SubChannelApp::DISPLACEMENT};

  // Get a list of the already existing AddAuxVariableAction
  const auto & aux_actions = _awh.getActions<AddAuxVariableAction>();

  for (auto & vn : var_names)
  {
    const std::string class_name = "AddAuxVariableAction";
    InputParameters params = _action_factory.getValidParams(class_name);
    params.set<MooseEnum>("family") = _fe_family;
    params.set<MooseEnum>("order") = _fe_order;

    std::shared_ptr<Action> action =
        std::static_pointer_cast<Action>(_action_factory.create(class_name, vn, params));

    //  Avoid trying (and failing) to override the user variable selection
    bool add_action = true;
    for (const auto aux_action : aux_actions)
      if (aux_action->name() == vn)
        add_action = false;

    if (add_action)
      _awh.addActionBlock(action);
  }
}
