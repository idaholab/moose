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

#include "MooseObject.h"
#include "SetupInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "FunctionInterface.h"
#include "TransientInterface.h"
#include "GeometricSearchInterface.h"
#include "MooseMesh.h"

class FaceFaceConstraint;
class FEProblem;

template<>
InputParameters validParams<FaceFaceConstraint>();

/**
 *
 */
class FaceFaceConstraint :
  public MooseObject,
  public SetupInterface,
  public CoupleableMooseVariableDependencyIntermediateInterface,
  public FunctionInterface,
  public TransientInterface,
  protected GeometricSearchInterface
{
public:
  FaceFaceConstraint(const std::string & name, InputParameters parameters);
  virtual ~FaceFaceConstraint();

  /**
   * Evaluate variables, compute q-points, etc.
   */
  virtual void reinit();

  /**
   * Computes the residual for the current element.
   */
  virtual void computeResidual();
  virtual void computeResidualSide();

  /**
   * Computes the jacobian for the current element.
   */
  virtual void computeJacobian(SparseMatrix<Number> & jacobian);

  /**
   * The variable number that this object operates on.
   */
  MooseVariable & variable();

  /**
   * Return a reference to the subproblem.
   */
  SubProblem & subProblem();

protected:
  virtual Real computeQpResidual() = 0;
  virtual Real computeQpResidualSide(Moose::ConstraintSideType side_type) = 0;
  virtual Real computeQpJacobianSide(Moose::ConstraintSideType side_type);

  FEProblem & _fe_problem;
  SubProblem & _subproblem;
  SystemBase & _sys;

  THREAD_ID _tid;

  Assembly & _assembly;
  MooseVariable & _var;
  MooseMesh & _mesh;
  unsigned int _dim;

  /// Boundary ID for the slave surface
  BoundaryID _slave;
  /// Boundary ID for the master surface
  BoundaryID _master;

  unsigned int _i, _j;

  unsigned int _qp;
  const MooseArray< Point > & _q_point;
  QBase * & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  MooseArray<Real> _JxW_lm;

  /**
   * Current element on the interface (i.e in the mortar space)
   */
  const Elem * & _current_elem;

  VariableTestValue _test;
  VariablePhiValue _phi;

  MooseVariable & _master_var;
  MooseVariable & _slave_var;

  /**
   * The values of Lagrange multipliers in quadrature points
   */
  VariableValue & _lambda;

  MooseMesh::MortarInterface & _iface;
  PenetrationLocator & _master_penetration_locator;
  PenetrationLocator & _slave_penetration_locator;

  /**
   * Values of the constrained variable on the master side
   */
  std::vector<Real> _u_master;
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
  /**
   * Values of shape function on the master side
   */
  const VariablePhiValue & _phi_master;

  /**
   * Values of the constrained variable on the slave side
   */
  std::vector<Real> _u_slave;
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
  /**
   * Values of shape function on the slave side
   */
  const VariablePhiValue & _phi_slave;
};


#endif /* FACEFACECONSTRAINT_H_ */
