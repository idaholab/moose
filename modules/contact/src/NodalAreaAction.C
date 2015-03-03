/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "NodalAreaAction.h"

#include "Factory.h"
#include "FEProblem.h"
#include "Parser.h"
#include "MooseApp.h"
#include "Conversion.h"

static unsigned int counter = 0;

template<>
InputParameters validParams<NodalAreaAction>()
{
  MooseEnum orders("FIRST SECOND THIRD FOURTH", "FIRST");

  InputParameters params = validParams<Action>();
  params.addParam<BoundaryName>("slave", "The slave surface");

  // Set this action to build "NodalArea"
  params.set<std::string>("type") = "NodalArea";
  return params;
}

NodalAreaAction::NodalAreaAction(const std::string & name, InputParameters params) :
  MooseObjectAction(name, params)
{
}

void
NodalAreaAction::act()
{
  std::string short_name(_name);
  // Chop off "Contact/"
  short_name.erase(0, 8);

  _moose_object_pars.set<std::vector<BoundaryName> >("boundary") = std::vector<BoundaryName>(1,getParam<BoundaryName>("slave"));
  _moose_object_pars.set<VariableName>("variable") = "nodal_area_"+short_name;

  _moose_object_pars.set<MultiMooseEnum>("execute_on") = "timestep_begin";
  _moose_object_pars.set<bool>("use_displaced_mesh") = true;

  _problem->addUserObject("NodalArea",
                          "nodal_area_object_" + Moose::stringify(counter++),
                          _moose_object_pars);
}
