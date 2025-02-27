#ifdef MFEM_ENABLED

#include "AddMFEMSolverAction.h"
#include "MFEMProblem.h"

registerMooseAction("MooseApp", AddMFEMSolverAction, "add_mfem_solver");

InputParameters
AddMFEMSolverAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Set the MooseMFEM solver and the solver options.");
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

  mfem_problem.addMFEMSolver(_type, _name, _moose_object_pars);
}

#endif
