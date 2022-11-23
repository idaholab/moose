//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// MOOSE includes
#include "AddElementalFieldAction.h"
#include "FEProblem.h"
#include "MooseMesh.h"

#include "libmesh/fe.h"

registerMooseAction("MooseApp", AddElementalFieldAction, "add_elemental_field_variable");

InputParameters
AddElementalFieldAction::validParams()
{
  InputParameters params = AddVariableAction::validParams();
  params.addClassDescription("Adds elemental auxiliary variable for adaptivity system.");
  params.ignoreParameter<std::string>("type");

  return params;
}

AddElementalFieldAction::AddElementalFieldAction(const InputParameters & params)
  : AddVariableAction(params)
{
}

void
AddElementalFieldAction::init()
{
  _moose_object_pars.set<MooseEnum>("order") = "CONSTANT";
  _moose_object_pars.set<MooseEnum>("family") = "MONOMIAL";

  _fe_type = FEType(CONSTANT, MONOMIAL);

  _type = "MooseVariableConstMonomial";

  _scalar_var = false;

  // Need static_cast to resolve overloads
  _problem_add_var_method = static_cast<void (FEProblemBase::*)(
      const std::string &, const std::string &, InputParameters &)>(&FEProblemBase::addAuxVariable);
}

void
AddElementalFieldAction::act()
{
  init();

  std::string variable = name();
  addVariable(variable);
}
