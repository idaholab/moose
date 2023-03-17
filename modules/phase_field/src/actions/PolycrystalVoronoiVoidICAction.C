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

registerMooseAction("PhaseFieldApp", PolycrystalVoronoiVoidICAction, "add_ic");

InputParameters
PolycrystalVoronoiVoidICAction::validParams()
{
  InputParameters params = Action::validParams();
  params += PolycrystalVoronoiVoidIC::actionParameters();
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.suppressParameter<VariableName>("variable");
  params.addRequiredParam<UserObjectName>(
      "polycrystal_ic_uo", "UserObject for obtaining the polycrystal grain structure.");
  params.addParam<FileName>(
      "file_name",
      "",
      "File containing grain centroids, if file_name is provided, the centroids "
      "from the file will be used.");
  params.addParam<std::vector<SubdomainName>>("block",
                                              "Block restriction for the initial condition");

  return params;
}

PolycrystalVoronoiVoidICAction::PolycrystalVoronoiVoidICAction(const InputParameters & params)
  : Action(params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base")),
    _file_name(getParam<FileName>("file_name"))
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
    poly_params.set<UserObjectName>("polycrystal_ic_uo") =
        getParam<UserObjectName>("polycrystal_ic_uo");

    // Add initial condition
    _problem->addInitialCondition(
        "PolycrystalVoronoiVoidIC", name() + "_" + Moose::stringify(op), poly_params);
  }
}
