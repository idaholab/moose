/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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

    MultiMooseEnum execute_options =
        MooseUtils::createExecuteOnEnum(3, EXEC_NONLINEAR, EXEC_TIMESTEP_END, EXEC_TIMESTEP_BEGIN);
    params.set<MultiMooseEnum>("execute_on") = execute_options;
    _problem->addAuxKernel("ContactPressureAux", name.str(), params);
  }
}
