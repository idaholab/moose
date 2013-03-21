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

template<>
InputParameters validParams<AddAuxVariableAction>()
{
  MooseEnum families(AddAuxVariableAction::getAuxVariableFamilies());
  MooseEnum orders(AddAuxVariableAction::getAuxVariableOrders());

  InputParameters params = validParams<Action>();
  params.addParam<MooseEnum>("family", families, "Specifies the family of FE shape functions to use for this variable");
  params.addParam<MooseEnum>("order", orders,  "Specifies the order of the FE shape function to use for this variable (additional orders not listed are allowed)");
  params.addParam<Real>("initial_condition", 0.0, "Specifies the initial condition for this variable");
  params.addParam<std::vector<SubdomainName> >("block", "The block id where this variable lives");

  return params;
}

AddAuxVariableAction::AddAuxVariableAction(const std::string & name, InputParameters params) :
    AddVariableAction(name, params)
{
}

MooseEnum
AddAuxVariableAction::getAuxVariableFamilies()
{
  return MooseEnum("LAGRANGE, MONOMIAL, SCALAR", "LAGRANGE");
}

MooseEnum
AddAuxVariableAction::getAuxVariableOrders()
{
  return MooseEnum("CONSTANT, FIRST, SECOND", "FIRST");
}
