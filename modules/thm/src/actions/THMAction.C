#include "THMAction.h"
#include "THMApp.h"
#include "Simulation.h"

template <>
InputParameters
validParams<THMAction>()
{
  InputParameters params = validParams<Action>();
  params.addPrivateParam<Simulation *>("_sim");
  return params;
}

THMAction::THMAction(InputParameters params)
  : Action(params), _simulation(*_app.parameters().getCheckedPointerParam<Simulation *>("_sim"))
{
  _pars.set<Simulation *>("_sim") = &_simulation;
}
