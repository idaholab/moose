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
  return params;
}

ADShaftConnectedMotorUserObject::ADShaftConnectedMotorUserObject(const InputParameters & params)
  : GeneralUserObject(params),
    ADShaftConnectableUserObjectInterface(this),
    _torque_fn(getFunction("torque")),
    _inertia_fn(getFunction("inertia"))
{
}

ADReal
ADShaftConnectedMotorUserObject::getTorque() const
{
  return _torque_fn.value(0.0, Point());
}

ADReal
ADShaftConnectedMotorUserObject::getMomentOfInertia() const
{
  return _inertia_fn.value(0.0, Point());
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
}
