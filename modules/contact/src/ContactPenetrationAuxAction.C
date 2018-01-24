//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactPenetrationAuxAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "MooseApp.h"

static unsigned int counter = 0;

template <>
InputParameters
validParams<ContactPenetrationAuxAction>()
{
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<Action>();
  params.addRequiredParam<BoundaryName>("master", "The master surface");
  params.addRequiredParam<BoundaryName>("slave", "The slave surface");
  params.addParam<MooseEnum>("order", orders, "The finite element order: FIRST, SECOND, etc.");
  return params;
}

ContactPenetrationAuxAction::ContactPenetrationAuxAction(const InputParameters & params)
  : Action(params),
    _master(getParam<BoundaryName>("master")),
    _slave(getParam<BoundaryName>("slave")),
    _order(getParam<MooseEnum>("order"))
{
}

void
ContactPenetrationAuxAction::act()
{
  if (!_problem->getDisplacedProblem())
  {
    mooseError("Contact requires updated coordinates.  Use the 'displacements = ...' line in the "
               "Mesh block.");
  }

  {
    InputParameters params = _factory.getValidParams("PenetrationAux");
    params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR};

    // Extract global params
    if (isParamValid("parser_syntax"))
      _app.parser().extractParams(getParam<std::string>("parser_syntax"), params);

    params.set<std::vector<BoundaryName>>("boundary") = {_slave};
    params.set<BoundaryName>("paired_boundary") = _master;
    params.set<AuxVariableName>("variable") = "penetration";
    params.set<MooseEnum>("order") = _order;

    params.set<bool>("use_displaced_mesh") = true;

    std::stringstream name;
    name << _name;
    name << "_contact_";
    name << counter++;
    _problem->addAuxKernel("PenetrationAux", name.str(), params);
  }
}
