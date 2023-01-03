//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShaftConnectedMotor.h"
#include "Shaft.h"

registerMooseObject("ThermalHydraulicsApp", ShaftConnectedMotor);

InputParameters
ShaftConnectedMotor::validParams()
{
  InputParameters params = Component::validParams();
  params += ShaftConnectable::validParams();
  params.addRequiredParam<FunctionName>("torque", "Driving torque supplied by the motor [kg-m^2]");
  params.addRequiredParam<FunctionName>("inertia", "Moment of inertia from the motor [N-m]");
  params.addParam<bool>("ad", true, "Use AD version or not");
  params.declareControllable("torque inertia");
  params.addClassDescription("Motor to drive a shaft component");
  return params;
}

ShaftConnectedMotor::ShaftConnectedMotor(const InputParameters & parameters)
  : Component(parameters),
    ShaftConnectable(this),
    _torque_fn_name(getParam<FunctionName>("torque")),
    _inertia_fn_name(getParam<FunctionName>("inertia"))
{
}

void
ShaftConnectedMotor::check() const
{
  checkShaftConnection(this);
}

void
ShaftConnectedMotor::addVariables()
{
}

void
ShaftConnectedMotor::addMooseObjects()
{
  makeFunctionControllableIfConstant(_torque_fn_name, "torque");
  makeFunctionControllableIfConstant(_inertia_fn_name, "inertia");

  const Shaft & shaft = getComponentByName<Shaft>(_shaft_name);
  const VariableName shaft_speed_var_name = shaft.getOmegaVariableName();

  const UserObjectName & uo_name = getShaftConnectedUserObjectName();
  if (getParam<bool>("ad"))
  {
    std::string class_name = "ADShaftConnectedMotorUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<FunctionName>("torque") = _torque_fn_name;
    params.set<FunctionName>("inertia") = _inertia_fn_name;
    params.set<std::vector<VariableName>>("shaft_speed") = {shaft_speed_var_name};
    getTHMProblem().addUserObject(class_name, uo_name, params);
  }
  else
  {
    std::string class_name = "ShaftConnectedMotorUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<FunctionName>("torque") = _torque_fn_name;
    params.set<FunctionName>("inertia") = _inertia_fn_name;
    params.set<std::vector<VariableName>>("shaft_speed") = {shaft_speed_var_name};
    getTHMProblem().addUserObject(class_name, uo_name, params);
  }
}
