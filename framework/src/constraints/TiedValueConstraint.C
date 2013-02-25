/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/
#include "TiedValueConstraint.h"

#include "SystemBase.h"

template<>
InputParameters validParams<TiedValueConstraint>()
{
  InputParameters params = validParams<NodeFaceConstraint>();
  params.addParam<Real>("scaling", 1, "scaling factor to be applied to constraint equations");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

TiedValueConstraint::TiedValueConstraint(const std::string & name, InputParameters parameters) :
    NodeFaceConstraint(name, parameters),
    _scaling(getParam<Real>("scaling")),
    _residual_copy(_sys.residualGhosted())
{}

Real
TiedValueConstraint::computeQpSlaveValue()
{
  return _u_master[_qp];
}

Real
TiedValueConstraint::computeQpResidual(Moose::ConstraintType type)
{
  Real scaling_factor = _var.scalingFactor();
  Real slave_resid = 0;
  Real retVal = 0;
  switch(type)
  {
  case Moose::Slave:
    retVal = (_u_slave[_qp] - _u_master[_qp])*_test_slave[_i][_qp]*_scaling;
    break;
  case Moose::Master:
    slave_resid = _residual_copy(_current_node->dof_number(0, _var.index(), 0)) / scaling_factor;
    retVal = slave_resid*_test_master[_i][_qp];
    break;
  default:
    break;
  }
  return retVal;
}

Real
TiedValueConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  Real scaling_factor = _var.scalingFactor();
  Real slave_jac = 0;
  Real retVal = 0;
  switch(type)
  {
  case Moose::SlaveSlave:
    retVal = _phi_slave[_j][_qp]*_test_slave[_i][_qp]*_scaling;
    break;
  case Moose::SlaveMaster:
    retVal = -_phi_master[_j][_qp]*_test_slave[_i][_qp]*_scaling;
    break;
  case Moose::MasterSlave:
    slave_jac = (*_jacobian)(_current_node->dof_number(0, _var.index(), 0), _connected_dof_indices[_j]);
    retVal = slave_jac*_test_master[_i][_qp] / scaling_factor;
    break;
  case Moose::MasterMaster:
    retVal = 0;
    break;
  }
  return retVal;
}

