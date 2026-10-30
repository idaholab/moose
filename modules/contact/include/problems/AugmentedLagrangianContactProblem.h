//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ReferenceResidualProblem.h"
#include "FEProblem.h"
#include "NodeFaceConstraint.h"
#include "MechanicalContactConstraint.h"
#include "AugmentedLagrangianContactProblemInterface.h"

/**
 * Class to manage nested solution for augmented Lagrange contact.
 * The AugmentedLagrangianContactProblem manages the nested solution procedure,
 * repeating the solution until convergence has been achieved, checking for convergence, and
 * updating the Lagrangian multipliers.
 */
template <class T>
class AugmentedLagrangianContactProblemTempl : public T,
                                               public AugmentedLagrangianContactProblemInterface
{
public:
  static InputParameters validParams();

  AugmentedLagrangianContactProblemTempl(const InputParameters & params);
  virtual ~AugmentedLagrangianContactProblemTempl() {}

  /**
   * Solves the problem at fixed Lagrange multipliers, then updates the multipliers and solves
   * again until the augmented Lagrangian constraints are met
   */
  virtual void solve(const unsigned int nl_sys_num) override;

  virtual bool solverSystemConverged(const unsigned int sys_num) override;

  /// The augmented Lagrangian loop sets the forced iterations of the default nonlinear convergence
  virtual bool onlyAllowDefaultNonlinearConvergence() const override { return true; }

  /// @return the total number of nonlinear iterations of all solves in the augmented Lagrangian loop
  virtual unsigned int nNonlinearIterations(const unsigned int nl_sys_num) const override;

  /// @return the total number of linear iterations of all solves in the augmented Lagrangian loop
  virtual unsigned int nLinearIterations(const unsigned int nl_sys_num) const override;

private:
  /**
   * Checks the augmented Lagrangian constraints and, if they are not met, updates the Lagrange
   * multipliers or, if the maximum number of updates has been reached, marks the solve as failed
   * @return Whether the constraints are met
   */
  bool updateAugmentedLagrangianMultipliers();

  /// Whether the augmented Lagrangian loop reached its maximum number of iterations
  bool _augmented_lagrangian_failed;

  /// Nonlinear iterations of all solves in the current augmented Lagrangian loop
  unsigned int _n_nonlinear_iterations;

  /// Linear iterations of all solves in the current augmented Lagrangian loop
  unsigned int _n_linear_iterations;
};

typedef AugmentedLagrangianContactProblemTempl<ReferenceResidualProblem>
    AugmentedLagrangianContactProblem;
typedef AugmentedLagrangianContactProblemTempl<FEProblem> AugmentedLagrangianContactFEProblem;
