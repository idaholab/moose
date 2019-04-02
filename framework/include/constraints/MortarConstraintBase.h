//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef MORTARCONSTRAINTBASE_H
#define MORTARCONSTRAINTBASE_H

// MOOSE includes
#include "Constraint.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "MooseMesh.h"
#include "MooseVariableInterface.h"

// Forward Declarations
class MortarConstraintBase;

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
   */
  virtual void computeResidual() = 0;

  /**
   * Method for computing the Jacobian
   */
  virtual void computeJacobian() = 0;

  /**
   * Returns the master lower dimensional subdomain id
   */
  virtual SubdomainID masterSubdomain() const = 0;

  /**
   * The variable number that this object operates on.
   */
  virtual MooseVariable * variable() = 0;
};

#endif /* MORTARCONSTRAINTBASE_H */
