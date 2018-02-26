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

template <>
InputParameters
validParams<AddSecondarySpeciesAction>()
{
  InputParameters params = validParams<AddAuxVariableAction>();
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
  for (auto i = beginIndex(_secondary_species); i < _secondary_species.size(); ++i)
    _problem->addAuxVariable(_secondary_species[i], _fe_type);
}
