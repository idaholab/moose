#include "AddProblemBuilderAction.h"

registerMooseAction("PlatypusApp", AddProblemBuilderAction, "add_mfem_problem_builder");

InputParameters
AddProblemBuilderAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Set the platypus ProblemBuilder to construct the problem to solve in the simulation.");
  return params;
}

AddProblemBuilderAction::AddProblemBuilderAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
AddProblemBuilderAction::act()
{
  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->setProblemBuilder();
}
