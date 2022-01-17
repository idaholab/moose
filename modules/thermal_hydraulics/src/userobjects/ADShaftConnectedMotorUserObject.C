#include "ADShaftConnectedMotorUserObject.h"

registerMooseObject("ThermalHydraulicsApp", ADShaftConnectedMotorUserObject);

InputParameters
ADShaftConnectedMotorUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += ADShaftConnectableUserObjectInterface::validParams();
  params.addRequiredParam<Real>("torque", "Driving torque supplied by the motor");
  params.addRequiredParam<Real>("inertia", "Moment of inertia from the motor");
  params.declareControllable("torque inertia");
  return params;
}

ADShaftConnectedMotorUserObject::ADShaftConnectedMotorUserObject(const InputParameters & params)
  : GeneralUserObject(params),
    ADShaftConnectableUserObjectInterface(this),
    _torque(getParam<Real>("torque")),
    _inertia(getParam<Real>("inertia"))
{
}

ADReal
ADShaftConnectedMotorUserObject::getTorque() const
{
  return _torque;
}

ADReal
ADShaftConnectedMotorUserObject::getMomentOfInertia() const
{
  return _inertia;
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
