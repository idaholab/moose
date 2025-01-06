#ifdef MFEM_ENABLED

#include "AddMFEMPreconditionerAction.h"
#include "MFEMProblem.h"

registerMooseAction("MooseApp", AddMFEMPreconditionerAction, "add_mfem_preconditioner");

InputParameters
AddMFEMPreconditionerAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a preconditioner to the MFEM problem.");
  return params;
}

AddMFEMPreconditionerAction::AddMFEMPreconditionerAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMPreconditionerAction::act()
{
  MFEMProblem & mfem_problem = static_cast<MFEMProblem &>(*_problem);

  mfem_problem.addMFEMPreconditioner(_type, _name, _moose_object_pars);
}

#endif
