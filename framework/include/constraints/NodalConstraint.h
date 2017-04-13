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

#ifndef NODALCONSTRAINT_H
#define NODALCONSTRAINT_H

// MOOSE includes
#include "Constraint.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"

// Forward Declarations
class NodalConstraint;

template <>
InputParameters validParams<NodalConstraint>();

class NodalConstraint : public Constraint,
                        public NeighborCoupleableMooseVariableDependencyIntermediateInterface
{
public:
  NodalConstraint(const InputParameters & parameters);

  /**
   * Get the list of master nodes
   * @return list of master nodes IDs
   */
  std::vector<dof_id_type> & getMasterNodeId() { return _master_node_vector; }

  /**
   * Get the list of connected slave nodes
   * @return list of slave node IDs
   */
  std::vector<dof_id_type> & getSlaveNodeId() { return _connected_nodes; }

  /**
   * Built the connectivity for this constraint
   */
  virtual void updateConnectivity();

  /**
   * Computes the nodal residual.
   */
  virtual void computeResidual(NumericVector<Number> & residual);

  /**
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian(SparseMatrix<Number> & jacobian);

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

  /// Value of the unknown variable this BC is action on
  const VariableValue & _u_slave;
  /// node IDs connected to the master node (slave nodes)
  std::vector<dof_id_type> _connected_nodes;
  /// node IDs of the master node
  std::vector<dof_id_type> _master_node_vector;
  /// Holds the current solution at the current quadrature point
  const VariableValue & _u_master;
  /// Specifies formulation type used to apply constraints
  Moose::ConstraintFormulationType _formulation;
  /**
   * When the slave node is constrained to move as a linear combination of the master nodes,
   * the coefficients associated with each master node is stored in _weights.
   */
  std::vector<Real> _weights;
  /// Counter for master and slave nodes
  unsigned int _i;
  unsigned int _j;
};

#endif
