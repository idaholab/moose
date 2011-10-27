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

// libMesh includes
#include "string_to_enum.h"

template<>
InputParameters validParams<TiedValueConstraint>()
{
  InputParameters params = validParams<NodeFaceConstraint>();
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

TiedValueConstraint::TiedValueConstraint(const std::string & name, InputParameters parameters) :
    NodeFaceConstraint(name, parameters),
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
//    std::cerr<<"slave: "<<_u_slave[_qp]<<", master:"<<_u_master[_qp]<<std::endl;
//  _u_master[_qp] = 0.5;
  switch(type)
  {
  case Moose::Slave:
    return (_u_slave[_qp] - _u_master[_qp])*_test_slave[_i][_qp];
  case Moose::Master:
    double slave_resid = _residual_copy(_current_node->dof_number(0, _var.number(), 0));
//    std::cerr<<"slave_resid: "<<slave_resid<<std::endl;
    return slave_resid*_test_master[_i][_qp];
  }
  return 0;
}

Real
TiedValueConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  double slave_jac = 0;
  switch(type)
  {
  case Moose::SlaveSlave:
    return _phi_slave[_j][_qp]*_test_slave[_i][_qp];
  case Moose::SlaveMaster:
    return -_phi_master[_j][_qp]*_test_slave[_i][_qp];
  case Moose::MasterSlave:
    slave_jac = (*_jacobian)(_current_node->dof_number(0, _var.number(), 0), _connected_dof_indices[_j]);
    return slave_jac*_test_master[_i][_qp];
  case Moose::MasterMaster:
    return 0;
  }
  return 0;
}

