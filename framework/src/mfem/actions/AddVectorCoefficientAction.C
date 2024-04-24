#include "AddVectorCoefficientAction.h"

registerMooseAction("PlatypusApp", AddVectorCoefficientAction, "add_mfem_coefficients");

InputParameters
AddVectorCoefficientAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a MFEM VectorCoefficient object to the simulation.");
  return params;
}

AddVectorCoefficientAction::AddVectorCoefficientAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddVectorCoefficientAction::act()
{
  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->addVectorCoefficient(_type, _name, _moose_object_pars);
}
