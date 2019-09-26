#include "AddComponentPhysicsAction.h"
#include "THMProblem.h"

registerMooseAction("THMApp", AddComponentPhysicsAction, "THM:add_component_physics");

template <>
InputParameters
validParams<AddComponentPhysicsAction>()
{
  InputParameters params = validParams<Action>();
  return params;
}

AddComponentPhysicsAction::AddComponentPhysicsAction(InputParameters params) : Action(params) {}

void
AddComponentPhysicsAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->addComponentPhysics();
}
