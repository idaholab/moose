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

template <>
InputParameters
validParams<CavityPressurePPAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<std::string>("output", "The name to use for the cavity pressure value");
  params.addParam<std::string>("output_initial_moles",
                               "The name to use when reporting the initial moles of gas");
  return params;
}

CavityPressurePPAction::CavityPressurePPAction(InputParameters params) : Action(params) {}

void
CavityPressurePPAction::act()
{
  std::string uo_name = _name + "UserObject";

  InputParameters params = _factory.getValidParams("CavityPressurePostprocessor");
  params.set<ExecFlagEnum>("execute_on") = {EXEC_INITIAL, EXEC_LINEAR};
  params.set<UserObjectName>("cavity_pressure_uo") = uo_name;
  params.set<std::string>("quantity") = "cavity_pressure";

  _problem->addPostprocessor("CavityPressurePostprocessor",
                             isParamValid("output") ? getParam<std::string>("output") : _name,
                             params);

  if (isParamValid("output_initial_moles"))
  {
    params.set<std::string>("quantity") = "initial_moles";
    _problem->addPostprocessor(
        "CavityPressurePostprocessor", getParam<std::string>("output_initial_moles"), params);
  }
}
