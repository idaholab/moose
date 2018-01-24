//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ThermalContactAuxVarsAction.h"
#include "FEProblem.h"
#include "AddVariableAction.h"

#include "libmesh/string_to_enum.h"

template <>
InputParameters
validParams<ThermalContactAuxVarsAction>()
{
  InputParameters params = validParams<Action>();
  params.addRequiredParam<NonlinearVariableName>("variable", "The variable for thermal contact");

  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  params.addParam<MooseEnum>("order", orders, "The finite element order");

  params.addParam<bool>(
      "quadrature", false, "Whether or not to use quadrature point based gap heat transfer");

  return params;
}

ThermalContactAuxVarsAction::ThermalContactAuxVarsAction(const InputParameters & params)
  : Action(params)
{
}

void
ThermalContactAuxVarsAction::act()
{
  // We need to add variables only once per variable name.  However, we don't know how many unique
  // variable names we will have.  So, we'll always add them.

  MooseEnum order = getParam<MooseEnum>("order");
  std::string family = "LAGRANGE";

  std::string penetration_var_name = "penetration";

  const bool quadrature = getParam<bool>("quadrature");
  if (quadrature)
  {
    order = "CONSTANT";
    family = "MONOMIAL";
    penetration_var_name = "qpoint_penetration";
  }

  _problem->addAuxVariable(
      penetration_var_name,
      FEType(Utility::string_to_enum<Order>(order), Utility::string_to_enum<FEFamily>(family)));
  _problem->addAuxVariable(
      getGapValueName(_pars),
      FEType(Utility::string_to_enum<Order>(order), Utility::string_to_enum<FEFamily>(family)));
}
