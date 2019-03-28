//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddPrimarySpeciesAction.h"
#include "FEProblem.h"

registerMooseAction("ChemicalReactionsApp", AddPrimarySpeciesAction, "add_variable");

template <>
InputParameters
validParams<AddPrimarySpeciesAction>()
{
  InputParameters params = validParams<AddVariableAction>();
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "primary_species", "The list of primary variables to add");
  params.addClassDescription("Adds Variables for all primary species");
  return params;
}

AddPrimarySpeciesAction::AddPrimarySpeciesAction(const InputParameters & params)
  : AddVariableAction(params),
    _vars(getParam<std::vector<NonlinearVariableName>>("primary_species")),
    _scaling(getParam<Real>("scaling"))
{
}

void
AddPrimarySpeciesAction::act()
{
  for (auto & var : _vars)
    _problem->addVariable(var, _fe_type, _scaling);
}
