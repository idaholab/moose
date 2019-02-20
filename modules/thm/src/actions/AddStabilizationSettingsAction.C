#include "AddStabilizationSettingsAction.h"

registerMooseAction("THMApp", AddStabilizationSettingsAction, "THM:add_stabilization");

template <>
InputParameters
validParams<AddStabilizationSettingsAction>()
{
  return validParams<THMObjectAction>();
}

AddStabilizationSettingsAction::AddStabilizationSettingsAction(InputParameters params)
  : THMObjectAction(params)
{
}

void
AddStabilizationSettingsAction::act()
{
  _problem->addUserObject(_type, _name, _moose_object_pars);
}
