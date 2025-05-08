#ifdef MFEM_ENABLED

#include "AddMFEMSubMeshTransferAction.h"

registerMooseAction("MooseApp", AddMFEMSubMeshTransferAction, "add_mfem_submesh_transfers");

InputParameters
AddMFEMSubMeshTransferAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a MFEM SubMesh object to the simulation.");
  return params;
}

AddMFEMSubMeshTransferAction::AddMFEMSubMeshTransferAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMSubMeshTransferAction::act()
{
  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->addUserObject(_type, _name, _moose_object_pars);
}

#endif
