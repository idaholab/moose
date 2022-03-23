//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ADShaftConnectedMotorUserObject.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ADShaftConnectedMotorUserObject);

InputParameters
ADShaftConnectedMotorUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += ADShaftConnectableUserObjectInterface::validParams();
  params.addRequiredParam<FunctionName>("torque", "Torque as a function of shaft speed");
  params.addRequiredParam<FunctionName>("inertia",
                                        "Moment of inertia as a function of shaft speed");
  params.addRequiredCoupledVar("shaft_speed", "Shaft speed");
  return params;
}

ADShaftConnectedMotorUserObject::ADShaftConnectedMotorUserObject(const InputParameters & params)
  : GeneralUserObject(params),
    ADShaftConnectableUserObjectInterface(this),
    _torque_fn(getFunction("torque")),
    _inertia_fn(getFunction("inertia")),
    _shaft_speed(adCoupledScalarValue("shaft_speed"))
{
}

ADReal
ADShaftConnectedMotorUserObject::getTorque() const
{
  const ADReal & shaft_speed = _shaft_speed[0];
  ADReal torque = _torque_fn.value(MetaPhysicL::raw_value(shaft_speed), Point());
  torque.derivatives() = _torque_fn.timeDerivative(MetaPhysicL::raw_value(shaft_speed), Point()) *
                         shaft_speed.derivatives();
  return torque;
}

ADReal
ADShaftConnectedMotorUserObject::getMomentOfInertia() const
{
  const ADReal & shaft_speed = _shaft_speed[0];
  ADReal inertia = _inertia_fn.value(MetaPhysicL::raw_value(shaft_speed), Point());
  inertia.derivatives() = _inertia_fn.timeDerivative(MetaPhysicL::raw_value(shaft_speed), Point()) *
                          shaft_speed.derivatives();
  return inertia;
}

void
ADShaftConnectedMotorUserObject::initialize()
{
}

void
ADShaftConnectedMotorUserObject::execute()
{
}

void
ADShaftConnectedMotorUserObject::finalize()
{
  ADShaftConnectableUserObjectInterface::finalize();
}
