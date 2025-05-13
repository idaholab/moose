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
class NodeElemConstraintBase : public Constraint,
public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
public NeighborMooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  NodeElemConstraintBase(const InputParameters & parameters);

  //fixme move
  /// Compute the value the secondary node should have at the beginning of a timestep.
  void computeSecondaryValue(NumericVector<Number> & current_solution);

    //fixme move
  /**
   * Whether or not this constraint should be applied.
   * @return bool true if this constraint is active, false otherwise
   */
   virtual bool shouldApply() { return true; }


    //fixme move
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

     //fixme move
  /**
   * The variable on the primary elem.
   * @return MooseVariable & a reference to the primary variable
   */
  virtual MooseVariable & primaryVariable() { return _primary_var; }

    //fixme move
  /**
   * The variable number that this object operates on.
   */
   const MooseVariable & variable() const override { return _var; }

protected:
  /// Compute the value the secondary node should have at the beginning of a timestep.
  virtual Real computeQpSecondaryValue() { return 0; } //fixme error

   MooseVariable & _var;
    /// Primary side variable
   MooseVariable & _primary_var;

  /**
   * Whether or not the secondary's residual should be overwritten.
   *
   * When this is true the secondary's residual as computed by the constraint will _replace_
   * the residual previously at that node for that variable.
   */
   bool _overwrite_secondary_residual;
//fixme move
public:
  SparseMatrix<Number> * _jacobian;
  /// dofs connected to the secondary node
  std::vector<dof_id_type> _connected_dof_indices;
  /// stiffness matrix holding primary-secondary jacobian
  DenseMatrix<Number> _Kne;
  /// stiffness matrix holding secondary-secondary jacobian
  DenseMatrix<Number> _Kee;
};
