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

  /// Boundary ID for the slave surface
  BoundaryID _slave_id;
  /// Boundary ID for the master surface
  BoundaryID _master_id;
  /// Subdomain ID for the slave surface
  SubdomainID _slave_subdomain_id;
  /// Subdomain ID for the master surface
  SubdomainID _master_subdomain_id;

  AutomaticMortarGeneration & _amg;

  MooseVariable & _lambda_var;
};

#endif /* REALMORTARCONSTRAINT_H */
