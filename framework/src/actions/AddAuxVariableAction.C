/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#include "AddAuxVariableAction.h"
#include "FEProblem.h"

template <>
InputParameters
validParams<AddAuxVariableAction>()
{
  MooseEnum families(AddAuxVariableAction::getAuxVariableFamilies());
  MooseEnum orders(AddAuxVariableAction::getAuxVariableOrders());

  InputParameters params = validParams<Action>();
  params += validParams<OutputInterface>();

  params.addParam<MooseEnum>(
      "family", families, "Specifies the family of FE shape functions to use for this variable");
  params.addParam<MooseEnum>("order",
                             orders,
                             "Specifies the order of the FE shape function to use "
                             "for this variable (additional orders not listed are "
                             "allowed)");
  params.addParam<Real>("initial_condition", "Specifies the initial condition for this variable");
  params.addParam<std::vector<SubdomainName>>("block", "The block id where this variable lives");

  return params;
}

AddAuxVariableAction::AddAuxVariableAction(InputParameters params) : AddVariableAction(params) {}

MooseEnum
AddAuxVariableAction::getAuxVariableFamilies()
{
  return MooseEnum("LAGRANGE MONOMIAL SCALAR", "LAGRANGE", true);
}

MooseEnum
AddAuxVariableAction::getAuxVariableOrders()
{
  return MooseEnum(
      "CONSTANT FIRST SECOND THIRD FOURTH FIFTH SIXTH SEVENTH EIGHTH NINTH", "FIRST", true);
}

void
AddAuxVariableAction::act()
{
  // Name of variable being added
  std::string var_name = name();

  // Blocks from the input
  std::set<SubdomainID> blocks = getSubdomainIDs();

  // Scalar variable
  if (_scalar_var)
    _problem->addAuxScalarVariable(var_name, _fe_type.order);

  // Non-scalar variable
  else
  {
    // Check that the order is valid (CONSTANT, FIRST, or SECOND)
    if (_fe_type.order > 9)
      mooseError("Non-scalar AuxVariables must be CONSTANT, FIRST, SECOND, THIRD, FOURTH, FIFTH, "
                 "SIXTH, SEVENTH, EIGHTH or NINTH order (",
                 _fe_type.order,
                 " supplied)");

    if (blocks.empty())
      _problem->addAuxVariable(var_name, _fe_type);
    else
      _problem->addAuxVariable(var_name, _fe_type, &blocks);
  }

  // Create the initial condition
  if (isParamValid("initial_condition"))
    createInitialConditionAction();
}
