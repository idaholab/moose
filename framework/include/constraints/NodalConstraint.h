//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
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

#include <map>
#include <set>

class MooseMesh;

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
   * Fill _primary_node_vector and _connected_nodes for this constraint.
   *
   * Called by updateConstrainedNodes() with both lists already cleared, and only to fill them:
   * the ghosting the lists imply is added afterwards by updateConstrainedNodes(). Put a secondary
   * node in _connected_nodes only on the process that owns it, for which localSecondaryNodes()
   * does the filtering.
   */
  virtual void updateConnectivity();

  /**
   * Rebuild the constrained node lists and the ghosting they require.
   *
   * Call this from the constructor of a derived class. It is called again on every mesh change,
   * since node ownership and the locally available elements both change under adaptivity and
   * repartitioning.
   */
  void updateConstrainedNodes();

  /**
   * Rebuilds the constrained nodes, which the mesh change may have redistributed.
   */
  virtual void meshChanged() override { updateConstrainedNodes(); }

  /**
   * Reinitialize the primary and secondary nodes on the SubProblem that owns this constraint's
   * variables.
   *
   * This must be called before computeResidual()/computeJacobian(). It is deliberately not part of
   * those methods because derived classes may override them without calling the base
   * implementation, which would silently skip the reinitialization.
   */
  void reinitConstraintNodes();

  /**
   * Computes the nodal residual.
   */
  virtual void computeResidual() override final
  {
    mooseError("NodalConstraint do not need computeResidual()");
  }
  virtual void computeResidual(const NumericVector<Number> & residual);

  /**
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian() override final
  {
    mooseError("NodalConstraint do not need computeJacobian()");
  }
  virtual void computeJacobian(const SparseMatrix<Number> & jacobian);

  /**
   * The variable number that this object operates on.
   */
  const MooseVariable & variable() const override { return _var; }

protected:
  /**
   * Gather and retain elements connected to the provided nodes on the provided mesh.
   */
  std::vector<dof_id_type> gatherAndRetainConnectedElems(MooseMesh & mesh,
                                                         const std::vector<dof_id_type> & node_ids);

  /**
   * Gather and retain one element per node on the reference mesh, and on the displaced mesh when
   * one exists, and mark them ghosted on the SubProblem.
   */
  void ghostConnectedElems(const std::vector<dof_id_type> & node_ids);

  /**
   * Select the nodes of 'node_ids' that this process owns, which are the secondary nodes it is
   * responsible for assembling.
   */
  std::vector<dof_id_type> localSecondaryNodes(const std::vector<dof_id_type> & node_ids) const;

  /// Elements this constraint retained on each distributed mesh during the previous mesh update.
  std::map<MooseMesh *, std::set<Elem *>> _retained_elems;

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
