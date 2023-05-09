//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalColoringICAction.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"
#include "PolycrystalICTools.h"

registerMooseAction("PhaseFieldApp", PolycrystalColoringICAction, "add_ic");

InputParameters
PolycrystalColoringICAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("Random Voronoi tessellation polycrystal action");
  params.addRequiredParam<unsigned int>("op_num", "number of order parameters to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addRequiredParam<UserObjectName>("polycrystal_ic_uo", "Optional: TODO");
  params.addParam<std::vector<SubdomainName>>("block",
                                              "Block restriction for the initial condition");

  return params;
}

PolycrystalColoringICAction::PolycrystalColoringICAction(const InputParameters & params)
  : Action(params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base"))
{
}

void
PolycrystalColoringICAction::act()
{
  // Loop through the number of order parameters
  for (unsigned int op = 0; op < _op_num; op++)
  {
    // Set parameters for BoundingBoxIC
    InputParameters poly_params = _factory.getValidParams("PolycrystalColoringIC");
    poly_params.set<VariableName>("variable") = _var_name_base + Moose::stringify(op);
    poly_params.set<unsigned int>("op_index") = op;
    poly_params.applySpecificParameters(parameters(), {"polycrystal_ic_uo", "block"});

    // Add initial condition
    _problem->addInitialCondition(
        "PolycrystalColoringIC", "PolycrystalColoringIC_" + Moose::stringify(op), poly_params);
  }
}
