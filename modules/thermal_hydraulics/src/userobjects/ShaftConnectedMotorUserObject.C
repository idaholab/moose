//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ShaftConnectedMotorUserObject.h"
#include "Function.h"

registerMooseObject("ThermalHydraulicsApp", ShaftConnectedMotorUserObject);

InputParameters
ShaftConnectedMotorUserObject::validParams()
{
  InputParameters params = GeneralUserObject::validParams();
  params += ShaftConnectableUserObjectInterface::validParams();
  params.addRequiredParam<FunctionName>("torque", "Torque as a function of shaft speed");
  params.addRequiredParam<FunctionName>("inertia",
                                        "Moment of inertia as a function of shaft speed");
  params.addRequiredCoupledVar("shaft_speed", "Shaft speed");
  return params;
}

ShaftConnectedMotorUserObject::ShaftConnectedMotorUserObject(const InputParameters & params)
  : GeneralUserObject(params),
    ShaftConnectableUserObjectInterface(this),
    _torque_fn(getFunction("torque")),
    _inertia_fn(getFunction("inertia")),
    _shaft_speed(coupledScalarValue("shaft_speed"))
{
}

Real
ShaftConnectedMotorUserObject::getTorque() const
{
  return _torque_fn.value(_shaft_speed[0], Point());
}

void
ShaftConnectedMotorUserObject::getTorqueJacobianData(DenseMatrix<Real> & /*jacobian_block*/,
                                                     std::vector<dof_id_type> & /*dofs_j*/) const
{
}

Real
ShaftConnectedMotorUserObject::getMomentOfInertia() const
{
  return _inertia_fn.value(_shaft_speed[0], Point());
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
