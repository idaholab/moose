#include "AddMFEMSolverAction.h"
#include "MFEMProblem.h"

InputParameters
AddMFEMSolverAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();

  params.addClassDescription("Set the platypus solver and the solver options.");

  return params;
}

AddMFEMSolverAction::AddMFEMSolverAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMSolverAction::act()
{
  MFEMProblem & mfem_problem = static_cast<MFEMProblem &>(*_problem);

  // TODO: - get the MFEMProblem to set the solver here with a public method.
}