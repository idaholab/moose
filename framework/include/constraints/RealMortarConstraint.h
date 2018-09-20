//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef REALMORTARCONSTRAINT_H
#define REALMORTARCONSTRAINT_H

// MOOSE includes
#include "Constraint.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MooseMesh.h"
#include "MooseVariableInterface.h"

// Forward Declarations
class RealMortarConstraint;
class FEProblemBase;

template <>
InputParameters validParams<RealMortarConstraint>();

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
class RealMortarConstraint : public Constraint,
                             public CoupleableMooseVariableDependencyIntermediateInterface,
                             public MooseVariableInterface<Real>
{
public:
  RealMortarConstraint(const InputParameters & parameters);

  virtual void computeResidual();

  virtual void computeJacobian();

protected:
  FEProblemBase & _fe_problem;
  SystemBase & _sys;
  unsigned int _dim;

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

  MooseVariable & _primal_var;
  MooseVariable & _lambda_var;

  /**
   * Values of the primal variable on the master side
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
  const VariableTestValue & _test_primal;
  const VariableTestGradient & _grad_test_primal;

  /**
   * Values of shape function on the master side
   */
  const VariablePhiValue & _phi_primal;

  /**
   * Values of the primal variable on the slave side
   */
  std::vector<Real> _u_slave;
  std::vector<RealGradient> _grad_u_slave;

  /**
   * Physical points on the slave side
   */
  std::vector<Point> _phys_points_slave;
  /**
   * Element on the slave side
   */
  const Elem * _elem_slave;

  /// Boundary ID for the slave surface
  BoundaryID _slave_id;
  /// Boundary ID for the master surface
  BoundaryID _master_id;
  /// Subdomain ID for the slave surface
  SubdomainID _slave_subdomain_id;
  /// Subdomain ID for the master surface
  SubdomainID _master_subdomain_id;

  AutomaticMortarGeneration & _amg;
};

#endif /* REALMORTARCONSTRAINT_H */
