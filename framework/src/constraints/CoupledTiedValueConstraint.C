//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CoupledTiedValueConstraint.h"

// MOOSE includes
#include "MooseVariable.h"
#include "SystemBase.h"

#include "libmesh/sparse_matrix.h"

template <>
InputParameters
validParams<CoupledTiedValueConstraint>()
{
  InputParameters params = validParams<NodeFaceConstraint>();
  params.addParam<Real>("scaling", 1, "scaling factor to be applied to constraint equations");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

CoupledTiedValueConstraint::CoupledTiedValueConstraint(const InputParameters & parameters)
  : NodeFaceConstraint(parameters),
    _scaling(getParam<Real>("scaling")),
    _residual_copy(_sys.residualGhosted())
{
}

Real
CoupledTiedValueConstraint::computeQpSlaveValue()
{
  return _u_master[_qp];
}

Real
CoupledTiedValueConstraint::computeQpResidual(Moose::ConstraintType type)
{
  Real scaling_factor = _var.scalingFactor();
  Real slave_resid = 0;
  Real retVal = 0;
  switch (type)
  {
    case Moose::Slave:
      retVal = (_u_slave[_qp] - _u_master[_qp]) * _test_slave[_i][_qp] * _scaling;
      break;
    case Moose::Master:
      slave_resid = _residual_copy(_current_node->dof_number(0, _var.number(), 0)) / scaling_factor;
      retVal = slave_resid * _test_master[_i][_qp];
      break;
    default:
      break;
  }
  return retVal;
}

Real
CoupledTiedValueConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  Real scaling_factor = _var.scalingFactor();
  Real slave_jac = 0;
  Real retVal = 0;
  switch (type)
  {
    case Moose::SlaveSlave:
      retVal = _phi_slave[_j][_qp] * _test_slave[_i][_qp] * _scaling;
      break;
    case Moose::SlaveMaster:
      retVal = 0;
      break;
    case Moose::MasterSlave:
      slave_jac =
          (*_jacobian)(_current_node->dof_number(0, _var.number(), 0), _connected_dof_indices[_j]);
      retVal = slave_jac * _test_master[_i][_qp] / scaling_factor;
      break;
    case Moose::MasterMaster:
      retVal = 0;
      break;
  }
  return retVal;
}

Real
CoupledTiedValueConstraint::computeQpOffDiagJacobian(Moose::ConstraintJacobianType type,
                                                     unsigned int jvar)
{
  Real retVal = 0;

  if (jvar == _master_var_num)
  {
    switch (type)
    {
      case Moose::SlaveSlave:
        retVal = 0;
        break;
      case Moose::SlaveMaster:
        retVal = -_phi_master[_j][_qp] * _test_slave[_i][_qp] * _scaling;
        break;
      case Moose::MasterSlave:
        retVal = 0;
        break;
      case Moose::MasterMaster:
        retVal = 0;
        break;
    }
  }

  return retVal;
}
