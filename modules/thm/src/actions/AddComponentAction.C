#include "AddComponentAction.h"
#include "THMProblem.h"

registerMooseAction("THMApp", AddComponentAction, "THM:add_component");

template <>
InputParameters
validParams<AddComponentAction>()
{
  InputParameters params = validParams<MooseObjectAction>();
  return params;
}

AddComponentAction::AddComponentAction(InputParameters params) : MooseObjectAction(params) {}

void
AddComponentAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
  {
    _moose_object_pars.set<THMProblem *>("_thm_problem") = thm_problem;
    thm_problem->addComponent(_type, _name, _moose_object_pars);
  }
}
