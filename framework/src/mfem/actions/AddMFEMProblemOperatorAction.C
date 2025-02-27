#ifdef MFEM_ENABLED

#include "AddMFEMProblemOperatorAction.h"

registerMooseAction("MooseApp", AddMFEMProblemOperatorAction, "add_mfem_problem_operator");

InputParameters
AddMFEMProblemOperatorAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription(
      "Set the ProblemOperator used in the MFEMExecutioner to solve the FE problem.");
  return params;
}

AddMFEMProblemOperatorAction::AddMFEMProblemOperatorAction(const InputParameters & parameters)
  : Action(parameters)
{
}

void
AddMFEMProblemOperatorAction::act()
{
  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->initProblemOperator();
}

#endif
