/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
