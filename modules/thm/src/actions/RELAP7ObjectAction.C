#include "RELAP7ObjectAction.h"
#include "RELAP7App.h"
#include "Simulation.h"

template <>
InputParameters
validParams<RELAP7ObjectAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

RELAP7ObjectAction::RELAP7ObjectAction(InputParameters params)
  : MooseObjectAction(params),
    _simulation(*_app.parameters().getCheckedPointerParam<Simulation *>("_sim"))
{
  _moose_object_pars.set<Simulation *>("_sim") = &_simulation;
}
