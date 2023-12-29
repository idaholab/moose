//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CavityPressurePPAction.h"
#include "Factory.h"
#include "FEProblem.h"

registerMooseAction("TensorMechanicsApp", CavityPressurePPAction, "add_postprocessor");

InputParameters
CavityPressurePPAction::validParams()
{
  InputParameters params = Action::validParams();
  params.addClassDescription("This Action creates a CavityPressurePostprocessor.");
  params.addParam<std::string>("output", "The name to use for the cavity pressure value");
  params.addParam<std::string>("output_initial_moles",
                               "The name to use when reporting the initial moles of gas");
  return params;
}

CavityPressurePPAction::CavityPressurePPAction(const InputParameters & params) : Action(params) {}

void
CavityPressurePPAction::act()
{
  InputParameters params = _factory.getValidParams("CavityPressurePostprocessor");

  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR};
  params.set<UserObjectName>("cavity_pressure_uo") = _name + "UserObject";
  params.set<MooseEnum>("quantity") = "cavity_pressure";

  _problem->addPostprocessor("CavityPressurePostprocessor",
                             isParamValid("output") ? getParam<std::string>("output") : _name,
                             params);

  if (isParamValid("output_initial_moles"))
  {
    params.set<MooseEnum>("quantity") = "initial_moles";
    _problem->addPostprocessor(
        "CavityPressurePostprocessor", getParam<std::string>("output_initial_moles"), params);
  }
}
