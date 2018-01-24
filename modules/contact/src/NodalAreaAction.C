//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "NodalAreaAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "MooseApp.h"
#include "Conversion.h"

static unsigned int counter = 0;

template <>
InputParameters
validParams<NodalAreaAction>()
{
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<Action>();
  params.addParam<BoundaryName>("slave", "The slave surface");

  // Set this action to build "NodalArea"
  params.set<std::string>("type") = "NodalArea";
  return params;
}

NodalAreaAction::NodalAreaAction(const InputParameters & params) : MooseObjectAction(params) {}

void
NodalAreaAction::act()
{
  _moose_object_pars.set<std::vector<BoundaryName>>("boundary") = {getParam<BoundaryName>("slave")};
  _moose_object_pars.set<std::vector<VariableName>>("variable") = {"nodal_area_" + _name};

  mooseAssert(_problem, "Problem pointer is NULL");
  _moose_object_pars.set<ExecFlagEnum>("execute_on", true) = {EXEC_INITIAL, EXEC_TIMESTEP_BEGIN};
  _moose_object_pars.set<bool>("use_displaced_mesh") = true;

  _problem->addUserObject(
      "NodalArea", "nodal_area_object_" + Moose::stringify(counter++), _moose_object_pars);
}
