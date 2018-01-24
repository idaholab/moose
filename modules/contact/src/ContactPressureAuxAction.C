//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ContactPressureAuxAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "MooseApp.h"
#include "Conversion.h"

static unsigned int counter = 0;

template <>
InputParameters
validParams<ContactPressureAuxAction>()
{
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<Action>();
  params.addRequiredParam<BoundaryName>("master", "The master surface");
  params.addRequiredParam<BoundaryName>("slave", "The slave surface");
  params.addParam<MooseEnum>("order", orders, "The finite element order: " + orders.getRawNames());
  return params;
}

ContactPressureAuxAction::ContactPressureAuxAction(const InputParameters & params)
  : Action(params),
    _master(getParam<BoundaryName>("master")),
    _slave(getParam<BoundaryName>("slave")),
    _order(getParam<MooseEnum>("order"))
{
}

void
ContactPressureAuxAction::act()
{
  if (!_problem->getDisplacedProblem())
  {
    mooseError("Contact requires updated coordinates.  Use the 'displacements = ...' line in the "
               "Mesh block.");
  }

  {
    InputParameters params = _factory.getValidParams("ContactPressureAux");

    // Extract global params
    if (isParamValid("parser_syntax"))
      _app.parser().extractParams(getParam<std::string>("parser_syntax"), params);

    params.set<std::vector<BoundaryName>>("boundary") = {_slave};
    params.set<BoundaryName>("paired_boundary") = _master;
    params.set<AuxVariableName>("variable") = "contact_pressure";
    params.addRequiredCoupledVar("nodal_area", "The nodal area");
    params.set<std::vector<VariableName>>("nodal_area") = {"nodal_area_" + _name};
    params.set<MooseEnum>("order") = _order;

    params.set<bool>("use_displaced_mesh") = true;

    std::stringstream name;
    name << _name;
    name << "_contact_pressure_";
    name << counter++;

    params.set<ExecFlagEnum>("execute_on",
                             true) = {EXEC_NONLINEAR, EXEC_TIMESTEP_END, EXEC_TIMESTEP_BEGIN};
    _problem->addAuxKernel("ContactPressureAux", name.str(), params);
  }
}
