#include "AddStabilizationSettingsAction.h"
#include "FEProblemBase.h"

registerMooseAction("THMApp", AddStabilizationSettingsAction, "THM:add_stabilization");

template <>
InputParameters
validParams<AddStabilizationSettingsAction>()
{
  return validParams<MooseObjectAction>();
}

AddStabilizationSettingsAction::AddStabilizationSettingsAction(InputParameters params)
  : MooseObjectAction(params)
{
}

void
AddStabilizationSettingsAction::act()
{
  _problem->addUserObject(_type, _name, _moose_object_pars);
}
