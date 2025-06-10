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

namespace libMesh
{
template <typename T>
class SparseMatrix;
}

/**
 * A NodeElemConstraintBase is used when you need to create constraints between
 * a secondary node and a primary element. It works by allowing you to modify the
 * residual and jacobian entries on the secondary node and the primary element.
 */
class NodeElemConstraintBase
  : public Constraint,
    public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
    public NeighborMooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  NodeElemConstraintBase(const InputParameters & parameters);
  ~NodeElemConstraintBase();

  /// Compute the value the secondary node should have at the beginning of a timestep.
  void computeSecondaryValue(NumericVector<Number> & current_solution);

  /**
   * Whether or not this constraint should be applied.
   * @return bool true if this constraint is active, false otherwise
   */
  virtual bool shouldApply() { return true; }

  /**
   * Whether or not the secondary's residual should be overwritten.
   * @return bool When this returns true the secondary's residual as computed by the constraint will
   * _replace_ the residual previously at that node for that variable.
   */
  virtual bool overwriteSecondaryResidual() const;

  /**
   * Whether or not the secondary's Jacobian row should be overwritten.
   * @return bool When this returns true the secondary's Jacobian row as computed by the constraint
   * will _replace_ the residual previously at that node for that variable.
   */
  virtual bool overwriteSecondaryJacobian() const { return overwriteSecondaryResidual(); };

  /**
   * The variable on the primary elem.
   * @return MooseVariable & a reference to the primary variable
   */
  virtual MooseVariable & primaryVariable() const { return _primary_var; }

  /**
   * The variable number that this object operates on.
   */
  const MooseVariable & variable() const override { return _var; }

protected:
  /// Compute the value the secondary node should have at the beginning of a timestep.
  virtual Real computeQpSecondaryValue() = 0;

  /// secondary node variable
  MooseVariable & _var;
  /// Primary side variable
  MooseVariable & _primary_var;

  /// secondary block id
  unsigned short _secondary;
  /// primary block id
  unsigned short _primary;

  /// current node being processed
  const Node * const & _current_node;
  /// current element being processed
  const Elem * const & _current_elem;

  /// Shape function on the secondary side.
  VariablePhiValue _phi_secondary;
  /// Shape function on the secondary side.  This will always only have one entry and that entry will always be "1"
  VariableTestValue _test_secondary;

  /// Side shape function.
  const VariablePhiValue & _phi_primary;

  /// Side test function
  const VariableTestValue & _test_primary;

  /// MooseMesh map of current nodes to the connected elements
  const std::map<dof_id_type, std::vector<dof_id_type>> & _node_to_elem_map;

  /// maps secondary node ids to primary element ids
  std::map<dof_id_type, dof_id_type> _secondary_to_primary_map;

  /**
   * Whether or not the secondary's residual should be overwritten.
   *
   * When this is true the secondary's residual as computed by the constraint will _replace_
   * the residual previously at that node for that variable.
   */
  bool _overwrite_secondary_residual;

public:
  /// system Jacobian, provides pre-constraint Jacobian for nonAD kinematic constraints
  const SparseMatrix<Number> * _jacobian;
  /// dofs connected to the secondary node
  std::vector<dof_id_type> _connected_dof_indices;
  /// stiffness matrix holding primary-secondary jacobian
  DenseMatrix<Number> _Kne;
  /// stiffness matrix holding secondary-secondary jacobian
  DenseMatrix<Number> _Kee;
};
