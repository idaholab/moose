/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
