/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/
#include "GapConductanceConstraint.h"

template <>
InputParameters
validParams<GapConductanceConstraint>()
{
  InputParameters params = validParams<FaceFaceConstraint>();
  params.addClassDescription(
      "Computes the residual and Jacobian contributions for the 'Lagrange Multiplier' "
      "implementation of the thermal contact problem. For more information, see the "
      "detailed description here: http://tinyurl.com/gmmhbe9");

  params.addRequiredParam<Real>("k", "Gap conductance");
  return params;
}

GapConductanceConstraint::GapConductanceConstraint(const InputParameters & parameters)
  : FaceFaceConstraint(parameters), _k(getParam<Real>("k"))
{
}

GapConductanceConstraint::~GapConductanceConstraint() {}

Real
GapConductanceConstraint::computeQpResidual()
{
  Real l = (_phys_points_master[_qp] - _phys_points_slave[_qp]).norm();
  return (_k * (_u_master[_qp] - _u_slave[_qp]) / l - _lambda[_qp]) * _test[_i][_qp];
}

Real
GapConductanceConstraint::computeQpResidualSide(Moose::ConstraintType res_type)
{
  switch (res_type)
  {
    case Moose::Master:
      return _lambda[_qp] * _test_master[_i][_qp];
    case Moose::Slave:
      return -_lambda[_qp] * _test_slave[_i][_qp];
    default:
      return 0;
  }
}

Real
GapConductanceConstraint::computeQpJacobian()
{
  return -_phi[_j][_qp] * _test[_i][_qp];
}

Real
GapConductanceConstraint::computeQpJacobianSide(Moose::ConstraintJacobianType jac_type)
{
  Real l = (_phys_points_master[_qp] - _phys_points_slave[_qp]).norm();
  switch (jac_type)
  {
    case Moose::MasterMaster:
      return (_k / l) * _phi[_j][_qp] * _test_master[_i][_qp];
    case Moose::MasterSlave:
      return -(_k / l) * _phi[_j][_qp] * _test_slave[_i][_qp];

    case Moose::SlaveMaster:
      return _phi[_j][_qp] * _test_master[_i][_qp];
    case Moose::SlaveSlave:
      return -_phi[_j][_qp] * _test_slave[_i][_qp];
    default:
      return 0;
  }
}
