//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef NODALFRICTIONALCONSTRAINT_H
#define NODALFRICTIONALCONSTRAINT_H

#include "NodalConstraint.h"

class NodalFrictionalConstraint;

template <>
InputParameters validParams<NodalFrictionalConstraint>();

class NodalFrictionalConstraint : public NodalConstraint
{
public:
  NodalFrictionalConstraint(const InputParameters & parameters);

  /**
   * Called on this object when the mesh changes
   */
  virtual void meshChanged() override;

  /**
   * Computes the nodal residual.
   */
  virtual void computeResidual(NumericVector<Number> & residual) override;

  /**
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian(SparseMatrix<Number> & jacobian) override;

protected:
  /**
   * Update the sets of nodes with constrained DOFs
   */
  void updateConstrainedNodes();

  /**
   * Computes the residual for the current slave node
   */
  virtual Real computeQpResidual(Moose::ConstraintType type) override;

  /**
   * Computes the jacobian for the constraint
   */
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;

  /// Holds the slave node set or side set
  BoundaryName _master_node_set_id;

  /// Holds the slave node set or side set
  BoundaryName _slave_node_set_id;

  /// Normal stiffness of spring
  const Real & _normal_force;

  /// Tangential stiffness of spring
  const Real & _tangential_penalty;

  /// Coefficient of friction
  const Real & _friction_coefficient;

  /// master node id connected to each slave node in _connected_nodes
  std::vector<dof_id_type> _master_conn;

  /// Old value of the constrainted variable on the slave nodes
  const VariableValue & _u_slave_old;

  /// Old value of the constrainted variable on the master nodes
  const VariableValue & _u_master_old;
};

#endif /* NODALFRICTIONALCONSTRAINT_H */
