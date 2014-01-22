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

//MOOSE includes
#include "Constraint.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"

//Forward Declarations
class NodalConstraint;

template<>
InputParameters validParams<NodalConstraint>();

class NodalConstraint :
  public Constraint,
  public NeighborCoupleableMooseVariableDependencyIntermediateInterface
{
public:
  NodalConstraint(const std::string & name, InputParameters parameters);
  virtual ~NodalConstraint();

  /**
   * Get the node ID of the master
   * @return node ID
   */
  unsigned int getMasterNodeId() const;

  /**
   * Get the list of connected slave nodes
   * @return list of slave node IDs
   */
  std::vector<unsigned int> & getSlaveNodeId() { return _connected_nodes; }

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
   * This is the virtual that derived classes should override for computing the residual on neighboring element.
   */
  virtual Real computeQpResidual(Moose::ConstraintType type) = 0;

  /**
   * This is the virtual that derived classes should override for computing the Jacobian on neighboring element.
   */
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) = 0;

  /// master node id
  unsigned int _master_node_id;

  /// Holds the reference to the master node
  const Node * & _master_node;
  /// Holds the reference to the current slave node
  const Node * & _slave_node;
  /// Value of the unknown variable this BC is action on
  VariableValue & _u_slave;
  /// node IDs connected to the master node (slave nodes)
  std::vector<unsigned int> _connected_nodes;

  /// Holds the current solution at the current quadrature point
  VariableValue & _u_master;
};

#endif
