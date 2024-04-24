#include "AddFormulationAction.h"

registerMooseAction("PlatypusApp", AddFormulationAction, "add_mfem_formulation");

InputParameters
AddFormulationAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Set the Hephaestus formulation to solve in the simulation.");
  return params;
}

AddFormulationAction::AddFormulationAction(const InputParameters & parameters)
  : MooseObjectAction(parameters)
{
}

void
AddFormulationAction::act()
{
  MFEMProblem * mfem_problem = dynamic_cast<MFEMProblem *>(_problem.get());
  if (mfem_problem)
    mfem_problem->setFormulation(_type, _name, _moose_object_pars);
}
