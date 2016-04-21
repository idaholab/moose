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

#ifndef ELEMELEMCONSTRAINT_H
#define ELEMELEMCONSTRAINT_H

// MOOSE includes
#include "Constraint.h"
#include "NeighborCoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MooseMesh.h"
#include "ElementPairInfo.h"

// Forward Declarations
class ElemElemConstraint;
class FEProblem;

template<>
InputParameters validParams<ElemElemConstraint>();

class ElemElemConstraint :
  public Constraint,
  public NeighborCoupleableMooseVariableDependencyIntermediateInterface
{
public:
  ElemElemConstraint(const InputParameters & parameters);
  virtual ~ElemElemConstraint();

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
  virtual void computeElemNeighResidual(Moose::DGResidualType type);

  /**
   * Computes the residual for the current side.
   */
  virtual void computeResidual();

  /**
   * Computes the element/neighbor-element/neighbor Jacobian
   */
  virtual void computeElemNeighJacobian(Moose::DGJacobianType type);

  /**
   * Computes the jacobian for the current side.
   */
  virtual void computeJacobian();

protected:
  FEProblem & _fe_problem;
  unsigned int _dim;

  const Elem * & _current_elem;

  /// The neighboring element
  const Elem * & _neighbor_elem;

  /// Quadrature points used in integration of constraint
  std::vector<Point> _constraint_q_point;
  /// Weights of quadrature points used in integration of constraint
  std::vector<Real> _constraint_weight;

  /// Indices for looping over DOFs
  unsigned int _i, _j;

  /// Holds the current solution at the current quadrature point
  const VariableValue & _u;

  /// Holds the current solution gradient at the current quadrature point
  const VariableGradient & _grad_u;
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

  /// Holds the current solution at the current quadrature point on the neighbor element
  const VariableValue & _u_neighbor;
  /// Holds the current solution gradient at the current quadrature point on the neighbor element
  const VariableGradient & _grad_u_neighbor;

  /**
   *  Compute the residual for one of the constraint quadrature points.  Must be overwritten by derived class.
   */
  virtual Real computeQpResidual(Moose::DGResidualType type) = 0;

  /**
   *  Compute the Jacobian for one of the constraint quadrature points.  Must be overwritten by derived class.
   */
  virtual Real computeQpJacobian(Moose::DGJacobianType type) = 0;
};

#endif /* ELEMELEMCONSTRAINT_H_ */
