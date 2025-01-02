#include "AddFESpaceAction.h"

registerMooseAction("MooseApp", AddFESpaceAction, "add_mfem_fespaces");

InputParameters
AddFESpaceAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a MFEM FESpace object to the simulation.");
  return params;
}

AddFESpaceAction::AddFESpaceAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddFESpaceAction::act()
{
  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->addFESpace(_type, _name, _moose_object_pars);
}
