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
#include "NodalConstraint.h"

#include "SystemBase.h"

template<>
InputParameters validParams<NodalConstraint>()
{
  InputParameters params = validParams<Constraint>();
  params.addRequiredParam<unsigned int>("master", "The ID of the master node");
  return params;
}

NodalConstraint::NodalConstraint(const std::string & name, InputParameters parameters) :
    Constraint(name, parameters),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(parameters, true),
    _master_node_id(getParam<unsigned int>("master")),
    _master_node(_assembly.node()),
    _slave_node(_assembly.nodeNeighbor()),
    _u_slave(_var.nodalSlnNeighbor()),
    _u_master(_var.nodalSln())
{
}

NodalConstraint::~NodalConstraint()
{
}

unsigned int
NodalConstraint::getMasterNodeId() const
{
  return _master_node_id;
}

void
NodalConstraint::computeResidual(NumericVector<Number> & residual)
{
  Real scaling_factor = _var.scalingFactor();
  _qp = 0;
  // master node
  dof_id_type & dof_idx = _var.nodalDofIndex();
  residual.add(dof_idx, scaling_factor * computeQpResidual(Moose::Master));
  // slave node
  dof_id_type & dof_idx_neighbor = _var.nodalDofIndexNeighbor();
  residual.add(dof_idx_neighbor, scaling_factor * computeQpResidual(Moose::Slave));
}

void
NodalConstraint::computeJacobian(SparseMatrix<Number> & jacobian)
{
  _qp = 0;
  dof_id_type & dof_idx = _var.nodalDofIndex();
  dof_id_type & dof_idx_neighbor = _var.nodalDofIndexNeighbor();

  Real scaling_factor = _var.scalingFactor();
  jacobian.add(dof_idx, dof_idx, scaling_factor * computeQpJacobian(Moose::MasterMaster));
  jacobian.add(dof_idx, dof_idx_neighbor, scaling_factor * computeQpJacobian(Moose::MasterSlave));
  jacobian.add(dof_idx_neighbor, dof_idx_neighbor, scaling_factor * computeQpJacobian(Moose::SlaveSlave));
  jacobian.add(dof_idx_neighbor, dof_idx, scaling_factor * computeQpJacobian(Moose::SlaveMaster));
}

void
NodalConstraint::updateConnectivity()
{
}
