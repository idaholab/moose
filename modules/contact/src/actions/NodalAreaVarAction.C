//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalAreaVarAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "MooseApp.h"
#include "libmesh/string_to_enum.h"

registerMooseAction("ContactApp", NodalAreaVarAction, "add_aux_variable");

template <>
InputParameters
validParams<NodalAreaVarAction>()
{
  MooseEnum orders("CONSTANT FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<Action>();
  params.addParam<MooseEnum>("order", orders, "The finite element order: " + orders.getRawNames());
  return params;
}

NodalAreaVarAction::NodalAreaVarAction(const InputParameters & params) : Action(params) {}

void
NodalAreaVarAction::act()
{
  auto var_params = _factory.getValidParams("MooseVariable");
  var_params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
  var_params.set<MooseEnum>("family") = "LAGRANGE";

  _problem->addAuxVariable("MooseVariable", "nodal_area_" + _name, var_params);
}
