#include "THMSetupQuadratureAction.h"
#include "THMProblem.h"

registerMooseAction("THMApp", THMSetupQuadratureAction, "setup_quadrature");

InputParameters
THMSetupQuadratureAction::validParams()
{
  InputParameters params = Action::validParams();
  return params;
}

THMSetupQuadratureAction::THMSetupQuadratureAction(InputParameters parameters) : Action(parameters)
{
}

void
THMSetupQuadratureAction::act()
{
  THMProblem * thm_problem = dynamic_cast<THMProblem *>(_problem.get());
  if (thm_problem)
    thm_problem->setupQuadrature();
}
