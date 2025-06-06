#ifdef MFEM_ENABLED

#include "AddMFEMSubMeshAction.h"

registerMooseAction("MooseApp", AddMFEMSubMeshAction, "add_mfem_submeshes");

InputParameters
AddMFEMSubMeshAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a MFEM SubMesh object to the simulation.");
  return params;
}

AddMFEMSubMeshAction::AddMFEMSubMeshAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddMFEMSubMeshAction::act()
{
  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->addSubMesh(_type, _name, _moose_object_pars);
}

#endif
