//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalConstraint.h"

class NodalStickConstraint : public NodalConstraint
{
public:
  NodalStickConstraint(const InputParameters & parameters);

  static InputParameters validParams();

  virtual void meshChanged() override;

  virtual void computeResidual(NumericVector<Number> & residual) override;
  virtual void computeJacobian(SparseMatrix<Number> & jacobian) override;
  using NodalConstraint::computeJacobian;
  using NodalConstraint::computeResidual;

protected:
  /**
   * Update the sets of nodes with constrained DOFs
   */
  void updateConstrainedNodes();

  virtual Real computeQpResidual(Moose::ConstraintType type) override;
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) override;

  /// Holds the secondary node set or side set
  BoundaryName _primary_boundary_id;

  /// Holds the secondary node set or side set
  BoundaryName _secondary_boundary_id;

  /// Tangential stiffness of spring in all directions
  const Real & _penalty;

  /// primary node id connected to each secondary node in _connected_nodes
  std::vector<dof_id_type> _primary_conn;
};
