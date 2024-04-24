#include "AddCoefficientAction.h"

registerMooseAction("PlatypusApp", AddCoefficientAction, "add_mfem_coefficients");

InputParameters
AddCoefficientAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a MFEM Coefficient object to the simulation.");
  return params;
}

AddCoefficientAction::AddCoefficientAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddCoefficientAction::act()
{
  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->addCoefficient(_type, _name, _moose_object_pars);
}
