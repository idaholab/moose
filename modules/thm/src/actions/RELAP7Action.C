#include "RELAP7Action.h"
#include "RELAP7App.h"
#include "Simulation.h"

template <>
InputParameters
validParams<RELAP7Action>()
{
  InputParameters params = validParams<Action>();
  params.addPrivateParam<Simulation *>("_sim");
  return params;
}

RELAP7Action::RELAP7Action(InputParameters params)
  : Action(params), _simulation(*_app.parameters().getCheckedPointerParam<Simulation *>("_sim"))
{
  _pars.set<Simulation *>("_sim") = &_simulation;
}
