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
  params.addClassDescription("Action to create ICs for polycrystal variables from a UserObject");
  params.addRequiredParam<unsigned int>("op_num", "number of order parameters to create");
  params.addRequiredParam<std::string>("var_name_base", "specifies the base name of the variables");
  params.addRequiredParam<UserObjectName>("polycrystal_ic_uo", "Optional: TODO");
  params.addParam<std::vector<SubdomainName>>("block",
                                              "Block restriction for the initial condition");
  params.addParam<bool>(
      "linearized_interface", false, "Whether to use linearized interface or the standard model");
  params.addParam<Real>("bound_value",
                        5.0,
                        "Bound value used to keep variable "
                        "between +/-bound. Must be positive.");
  params.addParamNamesToGroup("linearized_interface", "LinearizedInterface");
  params.addParamNamesToGroup("bound_value", "LinearizedInterface");

  return params;
}

PolycrystalColoringICAction::PolycrystalColoringICAction(const InputParameters & params)
  : Action(params),
    _op_num(getParam<unsigned int>("op_num")),
    _var_name_base(getParam<std::string>("var_name_base")),
    _linearized_interface(getParam<bool>("linearized_interface"))
{
}

void
PolycrystalColoringICAction::act()
{
  // Loop through the number of order parameters
  for (unsigned int op = 0; op < _op_num; op++)
  {
    std::string IC_type = "PolycrystalColoringIC";
    if (_linearized_interface)
      IC_type = "PolycrystalColoringICLinearizedInterface";

    // Set parameters for IC
    InputParameters poly_params = _factory.getValidParams(IC_type);
    poly_params.set<VariableName>("variable") = _var_name_base + Moose::stringify(op);
    poly_params.set<unsigned int>("op_index") = op;
    poly_params.applySpecificParameters(parameters(), {"polycrystal_ic_uo", "block"});
    poly_params.set<UserObjectName>("polycrystal_ic_uo") =
        getParam<UserObjectName>("polycrystal_ic_uo");
    if (_linearized_interface)
      poly_params.applySpecificParameters(parameters(),
                                          {"polycrystal_ic_uo", "block", "bound_value"});
    else
      poly_params.applySpecificParameters(parameters(), {"polycrystal_ic_uo", "block"});

    // Add initial condition
    _problem->addInitialCondition(IC_type, IC_type + Moose::stringify(op), poly_params);
  }
}
