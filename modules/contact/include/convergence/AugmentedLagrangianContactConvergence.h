//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AugmentedLagrangianContactProblemInterface.h"
#include "ReferenceResidualConvergence.h"
#include "DefaultNonlinearConvergence.h"

/**
 * Class to check convergence for the augmented Lagrangian contact problem.
 * @tparam T a convergence class type to use for such problems
 */
template <class T>
class AugmentedLagrangianContactConvergence : public T,
                                              public AugmentedLagrangianContactProblemInterface
{
public:
  static InputParameters validParams();

  AugmentedLagrangianContactConvergence(const InputParameters & params);

  virtual Convergence::MooseConvergenceStatus checkConvergence(unsigned int iter) override;

protected:
  using AugmentedLagrangianContactProblemInterface::_lagrangian_iteration_number;
  using AugmentedLagrangianContactProblemInterface::_maximum_number_lagrangian_iterations;
};

typedef AugmentedLagrangianContactConvergence<ReferenceResidualConvergence>
    AugmentedLagrangianContactReferenceConvergence;
typedef AugmentedLagrangianContactConvergence<DefaultNonlinearConvergence>
    AugmentedLagrangianContactFEProblemConvergence;
