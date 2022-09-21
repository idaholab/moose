//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AddAuxVariableAction.h"
#include "FEProblem.h"

registerMooseAction("MooseApp", AddAuxVariableAction, "add_aux_variable");

InputParameters
AddAuxVariableAction::validParams()
{
  InputParameters params = AddVariableAction::validParams();
  params.addClassDescription("Add auxiliary variable to the simulation.");
  return params;
}

AddAuxVariableAction::AddAuxVariableAction(const InputParameters & params)
  : AddVariableAction(params)
{
}

MooseEnum
AddAuxVariableAction::getAuxVariableFamilies()
{
  return MooseEnum("LAGRANGE MONOMIAL SCALAR LAGRANGE_VEC MONOMIAL_VEC", "LAGRANGE", true);
}

MooseEnum
AddAuxVariableAction::getAuxVariableOrders()
{
  return MooseEnum(
      "CONSTANT FIRST SECOND THIRD FOURTH FIFTH SIXTH SEVENTH EIGHTH NINTH", "FIRST", true);
}

void
AddAuxVariableAction::init()
{
  AddVariableAction::init();

  if (_fe_type.order > NINTH && !_scalar_var)
    mooseError("Non-scalar AuxVariables must be CONSTANT, FIRST, SECOND, THIRD, FOURTH, FIFTH, "
               "SIXTH, SEVENTH, EIGHTH or NINTH order (",
               _fe_type.order,
               " supplied)");

  // Need static_cast to resolve overloads
  _problem_add_var_method = static_cast<void (FEProblemBase::*)(
      const std::string &, const std::string &, InputParameters &)>(&FEProblemBase::addAuxVariable);
}
