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

// libMesh forward declarations
namespace libMesh
{
template <typename T>
class SparseMatrix;
}

/**
 * A NodeFaceConstraint is used when you need to create constraints between
 * two surfaces in a mesh.  It works by allowing you to modify the residual
 * and jacobian entries on "this" side (the node side, also referred to as
 * the secondary side) and the "other" side (the face side, also referred to as
 * the primary side)
 *
 * This is common for contact algorithms and other constraints.
 */
class NodeFaceConstraint : public Constraint,
                           public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
                           public NeighborMooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  NodeFaceConstraint(const InputParameters & parameters);
  virtual ~NodeFaceConstraint();

  /**
   * Compute the value the secondary node should have at the beginning of a timestep.
   */
  virtual void computeSecondaryValue(NumericVector<Number> & current_solution);

  /**
   * Computes the residual Nodal residual.
   */
  virtual void computeResidual() override;

  /**
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian() override;

  /**
   * Computes d-residual / d-jvar...
   */
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  /**
   * Gets the indices for all dofs connected to the constraint
   */
  virtual void getConnectedDofIndices(unsigned int var_num);

  /**
   * Whether or not this constraint should be applied.
   *
   * Get's called once per secondary node.
   */
  virtual bool shouldApply() { return true; }

  /**
   * Whether or not the secondary's residual should be overwritten.
   *
   * When this returns true the secondary's residual as computed by the constraint will _replace_
   * the residual previously at that node for that variable.
   */
  virtual bool overwriteSecondaryResidual();

  /**
   * Whether or not the secondary's Jacobian row should be overwritten.
   *
   * When this returns true the secondary's Jacobian row as computed by the constraint will
   * _replace_ the residual previously at that node for that variable.
   */
  virtual bool overwriteSecondaryJacobian() { return overwriteSecondaryResidual(); };

  /**
   * The variable on the Primary side of the domain.
   */
  virtual MooseVariable & primaryVariable() { return _primary_var; }

  /**
   * The primary boundary ID for this constraint.
   */
  BoundaryID primaryBoundary() const { return _primary; }

  /**
   * The secondary boundary ID for this constraint.
   */
  BoundaryID secondaryBoundary() const { return _secondary; }

  /**
   * The variable number that this object operates on.
   */
  const MooseVariable & variable() const override { return _var; }

  // TODO: Make this protected or add an accessor
  // Do the same for all the other public members
  SparseMatrix<Number> * _jacobian;

  Real secondaryResidual() const;

  void residualSetup() override;

protected:
  /**
   * Compute the value the secondary node should have at the beginning of a timestep.
   */
  virtual Real computeQpSecondaryValue() = 0;

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

  /**
   * This is the virtual that derived classes should override for computing the off-diag Jacobian.
   */
  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType /*type*/,
                                        unsigned int /*jvar*/)
  {
    return 0;
  }

  /// coupling interface:
  virtual const VariableValue & coupledSecondaryValue(const std::string & var_name,
                                                      unsigned int comp = 0)
  {
    return coupledValue(var_name, comp);
  }
  virtual const VariableValue & coupledSecondaryValueOld(const std::string & var_name,
                                                         unsigned int comp = 0)
  {
    return coupledValueOld(var_name, comp);
  }
  virtual const VariableValue & coupledSecondaryValueOlder(const std::string & var_name,
                                                           unsigned int comp = 0)
  {
    return coupledValueOlder(var_name, comp);
  }

  virtual const VariableGradient & coupledSecondaryGradient(const std::string & var_name,
                                                            unsigned int comp = 0)
  {
    return coupledGradient(var_name, comp);
  }
  virtual const VariableGradient & coupledSecondaryGradientOld(const std::string & var_name,
                                                               unsigned int comp = 0)
  {
    return coupledGradientOld(var_name, comp);
  }
  virtual const VariableGradient & coupledSecondaryGradientOlder(const std::string & var_name,
                                                                 unsigned int comp = 0)
  {
    return coupledGradientOlder(var_name, comp);
  }

  virtual const VariableSecond & coupledSecondarySecond(const std::string & var_name,
                                                        unsigned int comp = 0)
  {
    return coupledSecond(var_name, comp);
  }

  virtual const VariableValue & coupledPrimaryValue(const std::string & var_name,
                                                    unsigned int comp = 0)
  {
    return coupledNeighborValue(var_name, comp);
  }
  virtual const VariableValue & coupledPrimaryValueOld(const std::string & var_name,
                                                       unsigned int comp = 0)
  {
    return coupledNeighborValueOld(var_name, comp);
  }
  virtual const VariableValue & coupledPrimaryValueOlder(const std::string & var_name,
                                                         unsigned int comp = 0)
  {
    return coupledNeighborValueOlder(var_name, comp);
  }

  virtual const VariableGradient & coupledPrimaryGradient(const std::string & var_name,
                                                          unsigned int comp = 0)
  {
    return coupledNeighborGradient(var_name, comp);
  }
  virtual const VariableGradient & coupledPrimaryGradientOld(const std::string & var_name,
                                                             unsigned int comp = 0)
  {
    return coupledNeighborGradientOld(var_name, comp);
  }
  virtual const VariableGradient & coupledPrimaryGradientOlder(const std::string & var_name,
                                                               unsigned int comp = 0)
  {
    return coupledNeighborGradientOlder(var_name, comp);
  }

  virtual const VariableSecond & coupledPrimarySecond(const std::string & var_name,
                                                      unsigned int comp = 0)
  {
    return coupledNeighborSecond(var_name, comp);
  }

  /// Boundary ID for the secondary surface
  unsigned int _secondary;
  /// Boundary ID for the primary surface
  unsigned int _primary;

  MooseVariable & _var;

  const MooseArray<Point> & _primary_q_point;
  const QBase * const & _primary_qrule;

public:
  PenetrationLocator & _penetration_locator;

protected:
  /// current node being processed
  const Node * const & _current_node;
  const Elem * const & _current_primary;

  /// Value of the unknown variable this BC is action on
  const VariableValue & _u_secondary;
  /// Shape function on the secondary side.  This will always
  VariablePhiValue _phi_secondary;
  /// Shape function on the secondary side.  This will always only have one entry and that entry will always be "1"
  VariableTestValue _test_secondary;

  /// Primary side variable
  MooseVariable & _primary_var;

  /// Number for the primary variable
  unsigned int _primary_var_num;

  /// Side shape function.
  const VariablePhiValue & _phi_primary;
  /// Gradient of side shape function
  const VariablePhiGradient & _grad_phi_primary;

  /// Side test function
  const VariableTestValue & _test_primary;
  /// Gradient of side shape function
  const VariableTestGradient & _grad_test_primary;

  /// Holds the current solution at the current quadrature point
  const VariableValue & _u_primary;
  /// Holds the current solution gradient at the current quadrature point
  const VariableGradient & _grad_u_primary;

  /// DOF map
  const DofMap & _dof_map;

  const std::map<dof_id_type, std::vector<dof_id_type>> & _node_to_elem_map;

  /**
   * Whether or not the secondary's residual should be overwritten.
   *
   * When this is true the secondary's residual as computed by the constraint will _replace_
   * the residual previously at that node for that variable.
   */
  bool _overwrite_secondary_residual;

  /// JxW on the primary face
  const MooseArray<Real> & _primary_JxW;

  /// Whether the secondary residual has been computed
  bool _secondary_residual_computed;

  /// The value of the secondary residual
  Real _secondary_residual;

public:
  std::vector<dof_id_type> _connected_dof_indices;

  /// The Jacobian corresponding to the derivatives of the neighbor/primary residual with respect to
  /// the elemental/secondary degrees of freedom.  We want to manually manipulate Kne because of the
  /// dependence of the primary residuals on dofs from all elements connected to the secondary node
  /// (e.g. those held by _connected_dof_indices)
  DenseMatrix<Number> _Kne;

  /// The Jacobian corresponding to the derivatives of the elemental/secondary residual with respect to
  /// the elemental/secondary degrees of freedom.  We want to manually manipulate Kee because of the
  /// dependence of the secondary/primary residuals on // dofs from all elements connected to the secondary
  /// node (e.g. those held by _connected_dof_indices) // and because when we're overwriting the
  /// secondary residual we traditionally want to use a different // scaling factor from the one
  /// associated with interior physics
  DenseMatrix<Number> _Kee;

  /// The Jacobian corresponding to the derivatives of the elemental/secondary residual with respect to
  /// the neighbor/primary degrees of freedom.  We want to manually manipulate Ken because when we're
  /// overwriting the secondary residual we traditionally want to use a different scaling factor from the
  /// one associated with interior physics
  DenseMatrix<Number> _Ken;
};
