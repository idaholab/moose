//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// MOOSE includes
#include "Constraint.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"

class NodalConstraint : public Constraint,
                        public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
                        public NeighborMooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  NodalConstraint(const InputParameters & parameters);

  /**
   * Get the list of primary nodes
   * @return list of primary nodes IDs
   */
  std::vector<dof_id_type> & getPrimaryNodeId() { return _primary_node_vector; }

  /**
   * Get the list of connected secondary nodes
   * @return list of secondary node IDs
   */
  std::vector<dof_id_type> & getSecondaryNodeId() { return _connected_nodes; }

  /**
   * Built the connectivity for this constraint
   */
  virtual void updateConnectivity();

  /**
   * Computes the nodal residual.
   */
  virtual void computeResidual() override final
  {
    mooseError("NodalConstraint do not need computeResidual()");
  }
  virtual void computeResidual(NumericVector<Number> & residual);

  /**
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian() override final
  {
    mooseError("NodalConstraint do not need computeJacobian()");
  }
  virtual void computeJacobian(SparseMatrix<Number> & jacobian);

  /**
   * The variable number that this object operates on.
   */
  const MooseVariable & variable() const override { return _var; }

protected:
  /**
   * This is the virtual that derived classes should override for computing the residual on
   * neighboring element.
   */
  virtual Real computeQpResidual(Moose::ConstraintType type) = 0;

  /**
   * This is the virtual that derived classes should override for computing the Jacobian on
   * neighboring element.
   */
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) = 0;

  MooseVariable & _var;

  MooseVariable & _var_secondary;

  /// Value of the unknown variable this BC is action on
  const VariableValue & _u_secondary;
  /// node IDs connected to the primary node (secondary nodes)
  std::vector<dof_id_type> _connected_nodes;
  /// node IDs of the primary node
  std::vector<dof_id_type> _primary_node_vector;
  /// Holds the current solution at the current quadrature point
  const VariableValue & _u_primary;
  /// Specifies formulation type used to apply constraints
  Moose::ConstraintFormulationType _formulation;
  /**
   * When the secondary node is constrained to move as a linear combination of the primary nodes,
   * the coefficients associated with each primary node is stored in _weights.
   */
  std::vector<Real> _weights;
  /// Counter for primary and secondary nodes
  unsigned int _i;
  unsigned int _j;
};
