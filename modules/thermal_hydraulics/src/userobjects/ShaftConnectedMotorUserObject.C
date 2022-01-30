#include "ShaftConnectedMotorUserObject.h"

registerMooseObject("ThermalHydraulicsApp", ShaftConnectedMotorUserObject);

InputParameters
ShaftConnectedMotorUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += ShaftConnectableUserObjectInterface::validParams();
  params.addRequiredParam<Real>("torque", "Driving torque supplied by the motor");
  params.addRequiredParam<Real>("inertia", "Moment of inertia from the motor");
  params.declareControllable("torque inertia");
  return params;
}

ShaftConnectedMotorUserObject::ShaftConnectedMotorUserObject(const InputParameters & params)
  : GeneralUserObject(params),
    ShaftConnectableUserObjectInterface(this),
    _torque(getParam<Real>("torque")),
    _inertia(getParam<Real>("inertia"))
{
}

Real
ShaftConnectedMotorUserObject::getTorque() const
{
  return _torque;
}

void
ShaftConnectedMotorUserObject::getTorqueJacobianData(DenseMatrix<Real> & /*jacobian_block*/,
                                                     std::vector<dof_id_type> & /*dofs_j*/) const
{
}

Real
ShaftConnectedMotorUserObject::getMomentOfInertia() const
{
  return _inertia;
}

void
ShaftConnectedMotorUserObject::getMomentOfInertiaJacobianData(
    DenseMatrix<Real> & /*jacobian_block*/, std::vector<dof_id_type> & /*dofs_j*/) const
{
}

void
ShaftConnectedMotorUserObject::initialize()
{
}

void
ShaftConnectedMotorUserObject::execute()
{
}

void
ShaftConnectedMotorUserObject::finalize()
{
}
