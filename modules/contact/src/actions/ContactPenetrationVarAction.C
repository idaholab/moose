//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactPenetrationVarAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "MooseApp.h"
#include "libmesh/string_to_enum.h"

registerMooseAction("ContactApp", ContactPenetrationVarAction, "add_aux_variable");

template <>
InputParameters
validParams<ContactPenetrationVarAction>()
{
  MooseEnum orders("CONSTANT FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<Action>();
  params.addParam<MooseEnum>("order", orders, "The finite element order: FIRST, SECOND, etc.");
  return params;
}

ContactPenetrationVarAction::ContactPenetrationVarAction(const InputParameters & params)
  : Action(params)
{
}

void
ContactPenetrationVarAction::act()
{
  if (!_problem->getDisplacedProblem())
    mooseError("Contact requires updated coordinates.  Use the 'displacements = ...' line in the "
               "Mesh block.");

  auto var_params = _factory.getValidParams("MooseVariable");
  var_params.set<MooseEnum>("order") = getParam<MooseEnum>("order");
  var_params.set<MooseEnum>("family") = "LAGRANGE";

  _problem->addAuxVariable("MooseVariable", "penetration", var_params);
}
