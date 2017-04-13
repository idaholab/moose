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

#ifndef FACEFACECONSTRAINT_H
#define FACEFACECONSTRAINT_H

// MOOSE includes
#include "Constraint.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MooseMesh.h"

// Forward Declarations
class FaceFaceConstraint;
class FEProblemBase;

template <>
InputParameters validParams<FaceFaceConstraint>();

/**
 * User for mortar methods
 *
 * Indexing:
 *
 *              T_m             T_s         lambda
 *         +--------------+-------------+-------------+
 * T_m     |  K_1         |             | SlaveMaster |
 *         +--------------+-------------+-------------+
 * T_s     |              |  K_2        | SlaveSlave  |
 *         +--------------+-------------+-------------+
 * lambda  | MasterMaster | MasterSlave |             |
 *         +--------------+-------------+-------------+
 *
 */
class FaceFaceConstraint : public Constraint,
                           public CoupleableMooseVariableDependencyIntermediateInterface
{
public:
  FaceFaceConstraint(const InputParameters & parameters);

  /**
   * Evaluate variables, compute q-points, etc.
   */
  virtual void reinit();
  virtual void reinitSide(Moose::ConstraintType res_type);

  /**
   * Computes the residual for the current element.
   */
  virtual void computeResidual();
  /**
   * Computes residual contributions from master or slave side
   * @param side Master or slave side
   */
  virtual void computeResidualSide(Moose::ConstraintType side);

  /**
   * Computes the Jacobian for the current element (i.e element of the Mortar interface).
   */
  virtual void computeJacobian();
  /**
   * Computes Jacobian contributions from master or slave side
   * @param side Master or slave side
   */
  virtual void computeJacobianSide(Moose::ConstraintType side);

protected:
  virtual Real computeQpResidual() = 0;
  virtual Real computeQpResidualSide(Moose::ConstraintType res_type) = 0;
  virtual Real computeQpJacobian();
  virtual Real computeQpJacobianSide(Moose::ConstraintJacobianType jac_type);

  FEProblemBase & _fe_problem;
  unsigned int _dim;

  /// Boundary ID for the slave surface
  BoundaryID _slave;
  /// Boundary ID for the master surface
  BoundaryID _master;

  const MooseArray<Point> & _q_point;
  QBase *& _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  std::vector<Real> _JxW_lm;

  /**
   * Current element on the interface (i.e in the mortar space)
   */
  const Elem *& _current_elem;

  std::vector<std::vector<Real>> _test;
  std::vector<std::vector<Real>> _phi;

  MooseVariable & _master_var;
  MooseVariable & _slave_var;

  /**
   * The values of Lagrange multipliers in quadrature points
   */
  const VariableValue & _lambda;

  MooseMesh::MortarInterface & _iface;
  PenetrationLocator & _master_penetration_locator;
  PenetrationLocator & _slave_penetration_locator;

  /**
   * Values of the constrained variable on the master side
   */
  std::vector<Real> _u_master;
  std::vector<RealGradient> _grad_u_master;

  /**
   * Physical points on the master side
   */
  std::vector<Point> _phys_points_master;
  /**
   * Element on the master side
   */
  const Elem * _elem_master;
  /**
   * Values of test functions on the master side
   */
  const VariableTestValue & _test_master;
  const VariableTestGradient & _grad_test_master;

  /**
   * Values of shape function on the master side
   */
  const VariablePhiValue & _phi_master;

  /**
   * Values of the constrained variable on the slave side
   */
  std::vector<Real> _u_slave;
  std::vector<RealGradient> _grad_u_slave;

  /**
   * Physical points on the slave side
   */
  std::vector<Point> _phys_points_slave;
  /**
   * Element on the master side
   */
  const Elem * _elem_slave;
  /**
   * Values of test functions on the slave side
   */
  const VariableTestValue & _test_slave;
  const VariableTestGradient & _grad_test_slave;

  /**
   * Values of shape function on the slave side
   */
  const VariablePhiValue & _phi_slave;
};

#endif /* FACEFACECONSTRAINT_H */
