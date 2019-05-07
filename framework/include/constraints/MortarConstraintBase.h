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
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MooseMesh.h"
#include "MooseVariableInterface.h"
#include "TaggingInterface.h"

// Forward Declarations
class MortarConstraintBase;
class FEProblemBase;
namespace libMesh
{
class QBase;
}

template <>
InputParameters validParams<MortarConstraintBase>();

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
class MortarConstraintBase : public Constraint,
                             public CoupleableMooseVariableDependencyIntermediateInterface,
                             public MooseVariableInterface<Real>
{
public:
  MortarConstraintBase(const InputParameters & parameters);

  /**
   * Method for computing the residual
   * @param has_master Whether the mortar segment element projects onto the master face
   */
  virtual void computeResidual(bool has_master);

  /**
   * Method for computing the Jacobian
   * @param has_master Whether the mortar segment element projects onto the master face
   */
  virtual void computeJacobian(bool has_master);

  /**
   * compute the residual for the specified element type
   */
  virtual void computeResidual(Moose::MortarType mortar_type) = 0;

  /**
   * compute the residual for the specified element type
   */
  virtual void computeJacobian(Moose::MortarType mortar_type) = 0;

  /**
   * Returns the master lower dimensional subdomain id
   */
  SubdomainID masterSubdomain() const { return _master_subdomain_id; }

  /**
   * The variable number that this object operates on.
   */
  const MooseVariable * variable() const { return _var; }

private:
  /// Reference to the finite element problem
  FEProblemBase & _fe_problem;

  /// Boundary ID for the slave surface
  const BoundaryID _slave_id;

  /// Boundary ID for the master surface
  const BoundaryID _master_id;

  /// Subdomain ID for the slave surface
  const SubdomainID _slave_subdomain_id;

  /// Subdomain ID for the master surface
  const SubdomainID _master_subdomain_id;

protected:
  /// Pointer to the lagrange multipler variable. nullptr if none
  const MooseVariable * _var;

  /// Reference to the slave variable
  const MooseVariable & _slave_var;

  /// Reference to the master variable
  const MooseVariable & _master_var;

private:
  /// Whether to compute primal residuals
  const bool _compute_primal_residuals;

  /// Whether to compute lagrange multiplier residuals
  const bool _compute_lm_residuals;

  /// A dummy object useful for constructing _test when not using Lagrange multipliers
  const VariableTestValue _test_dummy;

protected:
  /// Whether the current mortar segment projects onto a face on the master side
  bool _has_master;

  /// the normals along the slave face
  const MooseArray<Point> & _normals;

  /// The element Jacobian times weights
  const std::vector<Real> & _JxW_msm;

  /// Member for handling change of coordinate systems (xyz, rz, spherical)
  const MooseArray<Real> & _coord;

  /// The quadrature rule
  const QBase * const & _qrule_msm;

  /// The shape functions corresponding to the lagrange multiplier variable
  const VariableTestValue & _test;

  /// The shape functions corresponding to the slave interior primal variable
  const VariableTestValue & _test_slave;

  /// The shape functions corresponding to the master interior primal variable
  const VariableTestValue & _test_master;

  /// The shape function gradients corresponding to the slave interior primal variable
  const VariableTestGradient & _grad_test_slave;

  /// The shape function gradients corresponding to the master interior primal variable
  const VariableTestGradient & _grad_test_master;

  /// The locations of the quadrature points on the interior slave elements
  const MooseArray<Point> & _phys_points_slave;

  /// The locations of the quadrature points on the interior master elements
  const MooseArray<Point> & _phys_points_master;
};

#define usingMortarConstraintBaseMembers                                                           \
  usingConstraintMembers;                                                                          \
  using ADMortarConstraint<compute_stage>::_phys_points_slave;                                     \
  using ADMortarConstraint<compute_stage>::_phys_points_master;                                    \
  using ADMortarConstraint<compute_stage>::_has_master;                                            \
  using ADMortarConstraint<compute_stage>::_test;                                                  \
  using ADMortarConstraint<compute_stage>::_test_slave;                                            \
  using ADMortarConstraint<compute_stage>::_test_master;                                           \
  using ADMortarConstraint<compute_stage>::_grad_test_slave;                                       \
  using ADMortarConstraint<compute_stage>::_grad_test_master;                                      \
  using ADMortarConstraint<compute_stage>::_normals
