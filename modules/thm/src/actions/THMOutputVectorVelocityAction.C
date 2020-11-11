#include "THMOutputVectorVelocityAction.h"
#include "THMProblem.h"

registerMooseAction("THMApp", THMOutputVectorVelocityAction, "THM:output_vector_velocity");

InputParameters
THMOutputVectorVelocityAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addParam<bool>(
      "velocity_as_vector", true, "True for vector-valued velocity, false for scalar velocity.");
  return params;
}

THMOutputVectorVelocityAction::THMOutputVectorVelocityAction(InputParameters params)
  : Action(params)
{
}

void
THMOutputVectorVelocityAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->setVectorValuedVelocity(getParam<bool>("velocity_as_vector"));
}
