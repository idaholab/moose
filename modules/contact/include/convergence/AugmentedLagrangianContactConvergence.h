//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
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
#include "ReferenceResidualConvergence.h"
#include "ResidualConvergence.h"

/**
 * Class to check convergence for the augmented Lagrangian contact problem.
 */
template <class T>
class AugmentedLagrangianContactConvergence : public T,
                                              public AugmentedLagrangianContactProblemInterface
{
public:
  static InputParameters validParams();

  AugmentedLagrangianContactConvergence(const InputParameters & params);

  virtual Convergence::MooseConvergenceStatus checkConvergence() override;

protected:
  FEProblemBase & _fe_problem;
  using AugmentedLagrangianContactProblemInterface::_lagrangian_iteration_number;
  using AugmentedLagrangianContactProblemInterface::_maximum_number_lagrangian_iterations;
};
typedef AugmentedLagrangianContactConvergence<ReferenceResidualConvergence>
    AugmentedLagrangianContactReferenceConvergence;
typedef AugmentedLagrangianContactConvergence<ResidualConvergence>
    AugmentedLagrangianContactFEProblemConvergence;
