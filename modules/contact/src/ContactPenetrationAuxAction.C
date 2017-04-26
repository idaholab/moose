/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
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
    MooseUtils::setExecuteOnFlags(params, {EXEC_INITIAL, EXEC_LINEAR});

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
