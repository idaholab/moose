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

#include "EqualValueNodalConstraint.h"

template <>
InputParameters
validParams<EqualValueNodalConstraint>()
{
  InputParameters params = validParams<NodalConstraint>();
  params.addRequiredParam<unsigned int>("master", "The ID of the master node");
  params.addRequiredParam<unsigned int>("slave", "The ID of the slave node");
  params.addRequiredParam<Real>("penalty", "The penalty used for the boundary term");
  return params;
}

EqualValueNodalConstraint::EqualValueNodalConstraint(const InputParameters & parameters)
  : NodalConstraint(parameters), _penalty(getParam<Real>("penalty"))
{
  _connected_nodes.push_back(getParam<unsigned int>("slave"));
  _master_node_vector.push_back(getParam<unsigned int>("master"));
}

EqualValueNodalConstraint::~EqualValueNodalConstraint() {}

Real
EqualValueNodalConstraint::computeQpResidual(Moose::ConstraintType type)
{
  switch (type)
  {
    case Moose::Master:
      return (_u_master[_j] - _u_slave[_i]) * _penalty;

    case Moose::Slave:
      return (_u_slave[_i] - _u_master[_j]) * _penalty;
  }

  return 0.;
}

Real
EqualValueNodalConstraint::computeQpJacobian(Moose::ConstraintJacobianType type)
{
  switch (type)
  {
    case Moose::MasterMaster:
      return _penalty;

    case Moose::MasterSlave:
      return -_penalty;

    case Moose::SlaveSlave:
      return _penalty;

    case Moose::SlaveMaster:
      return -_penalty;
  }

  return 0.;
}
