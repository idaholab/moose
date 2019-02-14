#include "THMObjectAction.h"
#include "THMApp.h"
#include "Simulation.h"

template <>
InputParameters
validParams<THMObjectAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

THMObjectAction::THMObjectAction(InputParameters params)
  : MooseObjectAction(params),
    _simulation(*_app.parameters().getCheckedPointerParam<Simulation *>("_sim"))
{
  _moose_object_pars.set<Simulation *>("_sim") = &_simulation;
}
