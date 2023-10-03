//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PhysicsDiscretization.h"
#include "AddVariableAction.h"

registerMooseObject("MooseApp", PhysicsDiscretization);

InputParameters
PhysicsDiscretization::validParams()
{
  InputParameters params = emptyInputParameters();
  params.addClassDescription(
      "Base class for indicating the discretization to use for your Physics");

  // Variable parameters
  MooseEnum families(AddVariableAction::getNonlinearVariableFamilies());
  MooseEnum orders(AddVariableAction::getNonlinearVariableOrders());
  params.addParam<std::vector<VariableName>>(
      "variables",
      {},
      "Variables used in the Physics. If unspecified, will be pulled in from the Physics");
  params.addParam<std::vector<MooseEnum>>(
      "family", {families}, "Specifies the family of FE shape functions to use for this variable");
  params.addParam<std::vector<MooseEnum>>("order",
                                          {orders},
                                          "Specifies the order of the FE shape function to use "
                                          "for this variable (additional orders not listed are "
                                          "allowed)");
  params.addParam<std::vector<Real>>("scaling",
                                     {1},
                                     "Specifies a scaling factor to apply to this variable. This "
                                     "is used for manual scaling of variables");
  params.registerBase("discretization");

  return params;
}

PhysicsDiscretization::PhysicsDiscretization(const InputParameters & parameters)
  : MooseObject(parameters),
    _var_names(getParam<std::vector<VariableName>>("variables")),
    _var_families(getParam<std::vector<MooseEnum>>("family")),
    _var_orders(getParam<std::vector<MooseEnum>>("order")),
    _var_scalings(getParam<std::vector<Real>>("scaling"))
{
}
