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

class ElementPairInfo;
class FEProblemBase;

class ElemElemConstraintBase
  : public Constraint,
    public NeighborCoupleableMooseVariableDependencyIntermediateInterface,
    public NeighborMooseVariableInterface<Real>
{
public:
  static InputParameters validParams();

  ElemElemConstraintBase(const InputParameters & parameters);

  /**
   * reinit element-element constraint
   */
  virtual void reinit(const ElementPairInfo & element_pair_info);

  /**
   * Set information needed for constraint integration
   */
  virtual void reinitConstraintQuadrature(const ElementPairInfo & element_pair_info);

  /**
   * Computes the residual for this element or the neighbor
   */
  virtual void computeElemNeighResidual(Moose::DGResidualType type) = 0;

  /**
   * Computes the residual for the current side.
   */
  virtual void computeResidual() override = 0;

  /**
   * Computes the element/neighbor-element/neighbor Jacobian
   */
  virtual void computeElemNeighJacobian(Moose::DGJacobianType type) = 0;

  /**
   * Computes the jacobian for the current side.
   */
  virtual void computeJacobian() override = 0;

  /**
   * Get the interface ID
   */
  unsigned int getInterfaceID() const { return _interface_id; };

  /**
   * The variable number that this object operates on.
   */
  const MooseVariable & variable() const override { return _var; }

protected:
  FEProblemBase & _fe_problem;
  unsigned int _dim;

  unsigned int _interface_id;

  MooseVariable & _var;

  const Elem * const & _current_elem;

  /// The neighboring element
  const Elem * const & _neighbor_elem;

  /// Quadrature points used in integration of constraint
  std::vector<Point> _constraint_q_point;
  /// Weights of quadrature points used in integration of constraint
  std::vector<Real> _constraint_weight;

  /// Indices for looping over DOFs
  unsigned int _i, _j;

  /// Shape function
  const VariablePhiValue & _phi;
  /// Shape function gradient
  const VariablePhiGradient & _grad_phi;

  /// Test function.
  const VariableTestValue & _test;
  /// Gradient of test function
  const VariableTestGradient & _grad_test;

  /// Neighbor shape function.
  const VariablePhiValue & _phi_neighbor;
  /// Gradient of neighbor shape function
  const VariablePhiGradient & _grad_phi_neighbor;

  /// Neighbor test function
  const VariableTestValue & _test_neighbor;
  /// Gradient of neighbor shape function
  const VariableTestGradient & _grad_test_neighbor;

  /**
   * Whether the element residual has been computed
   */
  bool _elem_residual_computed;

  /**
   * Whether the neighbor residual has been computed
   */
  bool _neighbor_residual_computed;
};
