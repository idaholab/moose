//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "PolycrystalVoronoiVoidICAction.h"
#include "PolycrystalVoronoiVoidIC.h"
#include "Factory.h"
#include "FEProblem.h"
#include "Conversion.h"

template <>
InputParameters
validParams<PolycrystalVoronoiVoidICAction>()
{
  InputParameters params = validParams<Action>();
  params += PolycrystalVoronoiVoidIC::actionParameters();
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.suppressParameter<VariableName>("variable");

  return params;
}

PolycrystalVoronoiVoidICAction::PolycrystalVoronoiVoidICAction(const InputParameters & params)
  : Action(params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base"))
{
}

void
PolycrystalVoronoiVoidICAction::act()
{
  // Loop through the number of order parameters
  for (unsigned int op = 0; op < _op_num; op++)
  {
    // Set parameters for BoundingBoxIC
    InputParameters poly_params = _factory.getValidParams("PolycrystalVoronoiVoidIC");
    poly_params.applyParameters(parameters());
    poly_params.set<unsigned int>("op_index") = op;
    poly_params.set<VariableName>("variable") = _var_name_base + Moose::stringify(op);
    poly_params.set<MooseEnum>("structure_type") = "grains";

    // Add initial condition
    _problem->addInitialCondition(
        "PolycrystalVoronoiVoidIC", name() + "_" + Moose::stringify(op), poly_params);
  }
}
