#include "AddProblemOperatorAction.h"

registerMooseAction("MooseApp", AddProblemOperatorAction, "add_mfem_problem_operator");

InputParameters
AddProblemOperatorAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Set the ProblemOperator used in the MFEMExecutioner to solve the FE problem.");
  return params;
}

AddProblemOperatorAction::AddProblemOperatorAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
AddProblemOperatorAction::act()
{
  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->initProblemOperator();
}
