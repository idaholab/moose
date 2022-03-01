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
 * A NodeElemConstraint is used when you need to create constraints between
 * a secondary node and a primary element. It works by allowing you to modify the
 * residual and jacobian entries on the secondary node and the primary element.
 */
class NodeElemConstraint : public Constraint,
                           public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
                           public NeighborMooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  NodeElemConstraint(const InputParameters & parameters);
  virtual ~NodeElemConstraint();

  /// Compute the value the secondary node should have at the beginning of a timestep.
  void computeSecondaryValue(NumericVector<Number> & current_solution);

  /// Computes the residual Nodal residual.
  virtual void computeResidual() override;

  /// Computes the jacobian for the current element.
  virtual void computeJacobian() override;

  /// Computes d-residual / d-jvar...
  virtual void computeOffDiagJacobian(unsigned int jvar) override;

  /// Gets the indices for all dofs connected to the constraint
  virtual void getConnectedDofIndices(unsigned int var_num);

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
  virtual bool overwriteSecondaryResidual();

  /**
   * Whether or not the secondary's Jacobian row should be overwritten.
   * @return bool When this returns true the secondary's Jacobian row as computed by the constraint
   * will _replace_ the residual previously at that node for that variable.
   */
  virtual bool overwriteSecondaryJacobian() { return overwriteSecondaryResidual(); };

  /**
   * The variable on the primary elem.
   * @return MooseVariable & a reference to the primary variable
   */
  virtual MooseVariable & primaryVariable() { return _primary_var; }

  /**
   * The variable number that this object operates on.
   */
  const MooseVariable & variable() const override { return _var; }

protected:
  /// prepare the _secondary_to_primary_map
  virtual void prepareSecondaryToPrimaryMap() = 0;

  /// Compute the value the secondary node should have at the beginning of a timestep.
  virtual Real computeQpSecondaryValue() = 0;

  /// This is the virtual that derived classes should override for computing the residual.
  virtual Real computeQpResidual(Moose::ConstraintType type) = 0;

  /// This is the virtual that derived classes should override for computing the Jacobian.
  virtual Real computeQpJacobian(Moose::ConstraintJacobianType type) = 0;

  /// This is the virtual that derived classes should override for computing the off-diag Jacobian.
  virtual Real computeQpOffDiagJacobian(Moose::ConstraintJacobianType /*type*/,
                                        unsigned int /*jvar*/)
  {
    return 0;
  }

  // coupling interface:
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

  /// secondary block id
  unsigned short _secondary;
  /// primary block id
  unsigned short _primary;

  MooseVariable & _var;

  const MooseArray<Point> & _primary_q_point;
  const QBase * const & _primary_qrule;

  /// current node being processed
  const Node * const & _current_node;
  const Elem * const & _current_elem;

  /// Value of the unknown variable on the secondary node
  const VariableValue & _u_secondary;
  /// old solution
  const VariableValue & _u_secondary_old;
  /// Shape function on the secondary side.
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
  /// Holds the old solution at the current quadrature point
  const VariableValue & _u_primary_old;
  /// Holds the current solution gradient at the current quadrature point
  const VariableGradient & _grad_u_primary;

  /// DOF map
  const DofMap & _dof_map;

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
  SparseMatrix<Number> * _jacobian;
  /// dofs connected to the secondary node
  std::vector<dof_id_type> _connected_dof_indices;
  /// stiffness matrix holding primary-secondary jacobian
  DenseMatrix<Number> _Kne;
  /// stiffness matrix holding secondary-secondary jacobian
  DenseMatrix<Number> _Kee;
};
