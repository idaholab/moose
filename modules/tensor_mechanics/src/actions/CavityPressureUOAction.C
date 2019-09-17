//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CavityPressureUOAction.h"
#include "Factory.h"
#include "FEProblem.h"

registerMooseAction("TensorMechanicsApp", CavityPressureUOAction, "add_user_object");

template <>
InputParameters
validParams<CavityPressureUOAction>()
{
  InputParameters params = validParams<Action>();
  params.addRangeCheckedParam<Real>(
      "initial_pressure", 0.0, "initial_pressure >= 0.0", "The initial pressure in the cavity");
  params.addParam<std::vector<PostprocessorName>>("material_input",
                                                  "The name of the postprocessor(s) that holds the "
                                                  "amount of material injected into the cavity");
  params.addRequiredRangeCheckedParam<Real>(
      "R", "R > 0.0", "The universal gas constant for the units used");
  params.addRequiredParam<PostprocessorName>(
      "temperature", "The name of the average temperature postprocessor value");
  params.addRangeCheckedParam<Real>(
      "initial_temperature", "initial_temperature > 0.0", "Initial temperature (optional)");
  params.addRequiredParam<std::vector<PostprocessorName>>(
      "volume",
      "The name of the postprocessor(s) that holds the value of the internal volume in the cavity");
  params.addParam<Real>(
      "startup_time",
      0.0,
      "The amount of time during which the pressure will ramp from zero to its true value");
  params.addParam<std::string>("output", "The name to use for the cavity pressure value");

  ExecFlagEnum exec_enum = MooseUtils::getDefaultExecFlagEnum();
  exec_enum = EXEC_LINEAR;
  params.addParam<ExecFlagEnum>("execute_on", exec_enum, exec_enum.getDocString());
  return params;
}

CavityPressureUOAction::CavityPressureUOAction(const InputParameters & params) : Action(params) {}

void
CavityPressureUOAction::act()
{
  InputParameters params = _factory.getValidParams("CavityPressureUserObject");

  params.applyParameters(parameters(), {"initial_temperature"});

  if (isParamValid("initial_temperature"))
    params.set<Real>("initial_temperature") = getParam<Real>("initial_temperature");

  _problem->addUserObject("CavityPressureUserObject", _name + "UserObject", params);
}
