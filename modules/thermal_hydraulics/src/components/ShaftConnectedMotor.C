#include "ShaftConnectedMotor.h"

registerMooseObject("ThermalHydraulicsApp", ShaftConnectedMotor);

InputParameters
ShaftConnectedMotor::validParams()
{
  InputParameters params = Component::validParams();
  params += ShaftConnectable::validParams();
  params.addRequiredParam<Real>("torque", "Driving torque supplied by the motor [kg-m^2]");
  params.addRequiredParam<Real>("inertia", "Moment of inertia from the motor [N-m]");
  params.addParam<bool>("ad", true, "Use AD version or not");
  params.addClassDescription("Motor to drive a shaft component");
  return params;
}

ShaftConnectedMotor::ShaftConnectedMotor(const InputParameters & parameters)
  : Component(parameters),
    ShaftConnectable(this),
    _torque(getParam<Real>("torque")),
    _inertia(getParam<Real>("inertia"))
{
}

void
ShaftConnectedMotor::addVariables()
{
}

void
ShaftConnectedMotor::addMooseObjects()
{
  const UserObjectName & uo_name = getShaftConnectedUserObjectName();
  if (getParam<bool>("ad"))
  {
    std::string class_name = "ADShaftConnectedMotorUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("torque") = _torque;
    params.set<Real>("inertia") = _inertia;
    _sim.addUserObject(class_name, uo_name, params);
    connectObject(params, uo_name, "torque");
    connectObject(params, uo_name, "inertia");
  }
  else
  {
    std::string class_name = "ShaftConnectedMotorUserObject";
    InputParameters params = _factory.getValidParams(class_name);
    params.set<Real>("torque") = _torque;
    params.set<Real>("inertia") = _inertia;
    _sim.addUserObject(class_name, uo_name, params);
    connectObject(params, uo_name, "torque");
    connectObject(params, uo_name, "inertia");
  }
}
