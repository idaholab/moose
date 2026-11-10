//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AugmentedLagrangianContactProblemInterface.h"
#include "FEProblemBase.h"
#include "Convergence.h"

InputParameters
AugmentedLagrangianContactProblemInterface::validParams()
{
  auto params = emptyInputParameters();
  params.addParam<int>("maximum_lagrangian_update_iterations",
                       100,
                       "Maximum number of update Lagrangian Multiplier iterations per step");
  return params;
}

AugmentedLagrangianContactProblemInterface::AugmentedLagrangianContactProblemInterface(
    const InputParameters & /*params*/, FEProblemBase & problem)
  : _problem(problem)
{
}

const AugmentedLagrangianContactConvergenceBase &
AugmentedLagrangianContactProblemInterface::getAugmentedLagrangianContactConvergence() const
{
  mooseAssert(_problem.numNonlinearSystems() == 1, "Multiple nonlinear systems are not supported.");

  const auto & name = _problem.getNonlinearConvergenceNames()[0];
  const auto * const convergence = dynamic_cast<const AugmentedLagrangianContactConvergenceBase *>(
      &_problem.getConvergence(name));
  if (!convergence)
    mooseError("The nonlinear convergence object '",
               name,
               "' is not an augmented Lagrangian contact convergence.");
  return *convergence;
}
