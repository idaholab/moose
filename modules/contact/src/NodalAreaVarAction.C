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
  _problem->addAuxVariable("nodal_area_" + _name,
                           FEType(Utility::string_to_enum<Order>(getParam<MooseEnum>("order")),
                                  Utility::string_to_enum<FEFamily>("LAGRANGE")));
}
