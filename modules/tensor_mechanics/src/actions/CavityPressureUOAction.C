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

template <>
InputParameters
validParams<CavityPressureUOAction>()
{
  InputParameters params = validParams<Action>();
  params.addParam<Real>("initial_pressure", 0, "The initial pressure in the cavity");
  params.addParam<std::vector<PostprocessorName>>("material_input",
                                                  "The name of the postprocessor(s) that holds the "
                                                  "amount of material injected into the cavity");
  params.addRequiredParam<Real>("R", "The universal gas constant for the units used");
  params.addRequiredParam<PostprocessorName>(
      "temperature", "The name of the average temperature postprocessor value");
  params.addParam<Real>("initial_temperature", "Initial temperature (optional)");
  params.addRequiredParam<PostprocessorName>("volume",
                                             "The name of the internal volume postprocessor value");
  params.addParam<Real>(
      "startup_time",
      0,
      "The amount of time during which the pressure will ramp from zero to its true value");
  params.addParam<std::string>("output", "The name to use for the cavity pressure value");

  ExecFlagEnum exec_enum = MooseUtils::getDefaultExecFlagEnum();
  exec_enum = EXEC_LINEAR;
  params.addParam<ExecFlagEnum>("execute_on", exec_enum, exec_enum.getDocString());
  return params;
}

CavityPressureUOAction::CavityPressureUOAction(const InputParameters & params)
  : Action(params),
    _initial_pressure(getParam<Real>("initial_pressure")),
    _material_input(getParam<std::vector<PostprocessorName>>("material_input")),
    _R(getParam<Real>("R")),
    _temperature(getParam<PostprocessorName>("temperature")),
    _volume(getParam<PostprocessorName>("volume")),
    _startup_time(getParam<Real>("startup_time"))
{
}

void
CavityPressureUOAction::act()
{
  std::string name = _name + "UserObject";

  InputParameters params = _factory.getValidParams("CavityPressureUserObject");
  params.set<ExecFlagEnum>("execute_on") = getParam<ExecFlagEnum>("execute_on");
  params.set<Real>("initial_pressure") = _initial_pressure;
  params.set<std::vector<PostprocessorName>>("material_input") = _material_input;
  params.set<Real>("R") = _R;
  params.set<PostprocessorName>("temperature") = _temperature;

  if (isParamValid("initial_temperature"))
    params.set<Real>("initial_temperature") = getParam<Real>("initial_temperature");

  params.set<PostprocessorName>("volume") = _volume;
  params.set<Real>("startup_time") = _startup_time;

  _problem->addUserObject("CavityPressureUserObject", name, params);
}
