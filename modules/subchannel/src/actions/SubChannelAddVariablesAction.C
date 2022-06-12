#include "SubChannelAddVariablesAction.h"
#include "ActionWarehouse.h"
#include "ActionFactory.h"
#include "AddVariableAction.h"
#include "SubChannelApp.h"

registerMooseAction("SubChannelApp", SubChannelAddVariablesAction, "meta_action");

InputParameters
SubChannelAddVariablesAction::validParams()
{
  InputParameters params = Action::validParams();
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
                                        SubChannelApp::DUCT_TEMPERATURE,
                                        SubChannelApp::DENSITY,
                                        SubChannelApp::VISCOSITY};

  for (auto & vn : var_names)
  {
    const std::string class_name = "AddAuxVariableAction";
    InputParameters params = _action_factory.getValidParams(class_name);
    params.set<MooseEnum>("family") = _fe_family;
    params.set<MooseEnum>("order") = _fe_order;

    std::shared_ptr<Action> action =
        std::static_pointer_cast<Action>(_action_factory.create(class_name, vn, params));

    _awh.addActionBlock(action);
  }
}
