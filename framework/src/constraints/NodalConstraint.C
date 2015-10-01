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
  return params;
}

NodalConstraint::NodalConstraint(const InputParameters & parameters) :
    Constraint(parameters),
    NeighborCoupleableMooseVariableDependencyIntermediateInterface(parameters, true, true),
    _master_node(_assembly.node()),
    _slave_node(_assembly.nodeNeighbor()),
    _u_slave(_var.nodalSlnNeighbor()),
    _u_master(_var.nodalSln())
{
}

NodalConstraint::~NodalConstraint()
{
}

void
NodalConstraint::computeResidual(NumericVector<Number> & residual)
{
  _qp = 0;

  // obtain the slave and master residual
  DenseVector<Number> re(1);
  re(0) = computeQpResidual(Moose::Master, residual)*_var.scalingFactor();
  DenseVector<Number> neighbor_re(1);
  neighbor_re(0) = computeQpResidual(Moose::Slave, residual)*_var.scalingFactor();

  // obtain slave and master dof_index
  std::vector<dof_id_type> slavedof(1,_var.nodalDofIndexNeighbor());
  std::vector<dof_id_type> masterdof(1,_var.nodalDofIndex());

  // Add the slave and master residual to the cache along with the corresponding dof indices
  _assembly.cacheResidualNodes(re, masterdof);
  _assembly.cacheResidualNodes(neighbor_re, slavedof);
}

void
NodalConstraint::computeJacobian(SparseMatrix<Number> & jacobian)
{
  // Calculate Jacobian enteries and cache those entries along with the row and column index
  _qp = 0;
  DenseMatrix<Number> _Kee(1,1);
 // _Kee.resize(1,1);
  std::vector<dof_id_type> slavedof(1,_var.nodalDofIndexNeighbor());
  std::vector<dof_id_type> masterdof(1,_var.nodalDofIndex());
   //Master-Master
  _Kee(0,0)= computeQpJacobian(Moose::MasterMaster, jacobian);
  _assembly.cacheJacobianBlock(_Kee, masterdof, masterdof, _var.scalingFactor());
   //Master-Slave
  _Kee(0,0) = computeQpJacobian(Moose::MasterSlave, jacobian);
  _assembly.cacheJacobianBlock(_Kee, masterdof, slavedof, _var.scalingFactor());
   //Slave-Master
  _Kee(0,0) = computeQpJacobian(Moose::SlaveMaster, jacobian);
  _assembly.cacheJacobianBlock(_Kee, slavedof, masterdof, _var.scalingFactor());
   //Slave-Slave
  _Kee(0,0) = computeQpJacobian(Moose::SlaveSlave, jacobian);
  _assembly.cacheJacobianBlock(_Kee, slavedof, slavedof, _var.scalingFactor());
}

void
NodalConstraint::updateConnectivity()
{
}
