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
   * Built the connectivity for this constraint
   */
  virtual void updateConnectivity();

  virtual bool usesConstraintRows() const override { return _formulation == Moose::Rows; }

  virtual void addConstraintRows(libMesh::DofMap & dof_map) const override;

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
   * Add one degree of freedom constraint row per secondary node, tying the secondary variable at
   * that node to the weighted sum of the primary variable at the primary nodes.
   *
   * Derived classes call this from their addConstraintRows() with the secondary nodes they hold at
   * that moment, which is not necessarily _connected_nodes: libMesh rebuilds the rows during
   * EquationSystems::reinit(), before MOOSE notifies this object that the mesh changed.
   *
   * This method is collective. Every rank adds the rows of the secondary nodes it has, and every
   * problem it finds is gathered so that all the ranks raise the same error instead of one rank
   * erroring on its own.
   *
   * @param dof_map The DofMap that receives the rows
   * @param secondary_nodes The IDs of the secondary nodes to constrain. An ID this rank does not
   * have, and a node that is also a primary node, are skipped
   */
  void addTieRows(libMesh::DofMap & dof_map,
                  const std::vector<dof_id_type> & secondary_nodes) const;

  /**
   * @return The IDs of the nodes of the boundary \p boundary_name that this rank owns
   *
   * The rows of a live boundary are built from this at call time, because libMesh rebuilds them
   * inside EquationSystems::reinit(), before MOOSE notifies this object that the mesh changed
   */
  std::vector<dof_id_type> ownedBoundaryNodes(const BoundaryName & boundary_name) const;

  /**
   * Error out when the 'penalty' parameter is missing and this constraint is enforced with a
   * residual. The rows formulation enforces the constraint exactly, so it needs no penalty.
   */
  void checkPenaltyParam() const;

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
