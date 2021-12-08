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

InputParameters
AddPrimarySpeciesAction::validParams()
{
  InputParameters params = AddVariableAction::validParams();
  params.addRequiredParam<std::vector<NonlinearVariableName>>(
      "primary_species", "The list of primary variables to add");
  params.addClassDescription("Adds Variables for all primary species");
  return params;
}

AddPrimarySpeciesAction::AddPrimarySpeciesAction(const InputParameters & params)
  : AddVariableAction(params),
    _vars(getParam<std::vector<NonlinearVariableName>>("primary_species")),
    _scaling(isParamValid("scaling") ? getParam<std::vector<Real>>("scaling")
                                     : std::vector<Real>(1, 1.0))
{
}

void
AddPrimarySpeciesAction::act()
{
  auto fe_type = AddVariableAction::feType(_pars);
  auto type = AddVariableAction::variableType(fe_type);
  auto var_params = _factory.getValidParams(type);

  var_params.applySpecificParameters(_pars, {"family", "order", "scaling"});

  for (auto & var : _vars)
    _problem->addVariable(type, var, var_params);
}
