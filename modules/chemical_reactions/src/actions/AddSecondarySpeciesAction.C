//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddSecondarySpeciesAction.h"
#include "FEProblem.h"

registerMooseAction("ChemicalReactionsApp", AddSecondarySpeciesAction, "add_aux_variable");

InputParameters
AddSecondarySpeciesAction::validParams()
{
  InputParameters params = AddAuxVariableAction::validParams();
  params.addParam<std::vector<AuxVariableName>>("secondary_species",
                                                "The list of secondary species to add");
  params.addClassDescription("Adds AuxVariables for all secondary species");
  return params;
}

AddSecondarySpeciesAction::AddSecondarySpeciesAction(const InputParameters & params)
  : AddAuxVariableAction(params),
    _secondary_species(getParam<std::vector<AuxVariableName>>("secondary_species"))
{
}

void
AddSecondarySpeciesAction::act()
{
  auto fe_type = AddVariableAction::feType(_pars);
  auto type = AddVariableAction::variableType(fe_type);
  auto var_params = _factory.getValidParams(type);

  var_params.applySpecificParameters(_pars, {"family", "order"});

  for (auto & secondary_specimen : _secondary_species)
    _problem->addAuxVariable(type, secondary_specimen, var_params);
}
