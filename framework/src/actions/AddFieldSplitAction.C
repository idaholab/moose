//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "AddFieldSplitAction.h"
#include "FEProblem.h"
#include "NonlinearSystemBase.h"

registerMooseAction("MooseApp", AddFieldSplitAction, "add_field_split");

InputParameters
AddFieldSplitAction::validParams()
{
  InputParameters params = MooseObjectAction::validParams();
  params.addClassDescription("Add a Split object to the simulation.");
  params.addParam<std::string>("type", "Split", "Classname of the split object");
  params.addParam<std::vector<NonlinearVariableName>>("vars", "variables for this field");
  params.addParam<MultiMooseEnum>(
      "petsc_options", Moose::PetscSupport::getCommonPetscFlags(), "Singleton PETSc options");
  params.addParam<MultiMooseEnum>("petsc_options_iname",
                                  Moose::PetscSupport::getCommonPetscKeys(),
                                  "Names of PETSc name/value pairs");
  params.addParam<std::vector<std::string>>(
      "petsc_options_value",
      "Values of PETSc name/value pairs (must correspond with \"petsc_options_iname\"");
  return params;
}

AddFieldSplitAction::AddFieldSplitAction(const InputParameters & params) : MooseObjectAction(params)
{
}

void
AddFieldSplitAction::act()
{
  _problem->getNonlinearSystemBase().addSplit(_type, _name, _moose_object_pars);
}
