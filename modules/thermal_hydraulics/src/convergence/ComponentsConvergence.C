//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ComponentsConvergence.h"
#include "THMProblem.h"
#include "Component.h"

registerMooseObject("ThermalHydraulicsApp", ComponentsConvergence);

InputParameters
ComponentsConvergence::validParams()
{
  InputParameters params = IterationCountConvergence::validParams();

  params.addClassDescription("Assesses convergence of all Component objects in a simulation.");

  return params;
}

ComponentsConvergence::ComponentsConvergence(const InputParameters & parameters)
  : IterationCountConvergence(parameters),
    _thm_problem(
        dynamic_cast<THMProblem *>(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")))
{
  if (!_thm_problem)
    mooseError("ComponentsConvergence only works with THMProblem.");
}

void
ComponentsConvergence::initialSetup()
{
  IterationCountConvergence::initialSetup();

  for (const auto & comp : _thm_problem->getComponents())
  {
    const auto nl_conv = comp->getNonlinearConvergence();
    if (nl_conv)
      _convergence_objects.push_back(nl_conv);
  }
}

Convergence::MooseConvergenceStatus
ComponentsConvergence::checkConvergenceInner(unsigned int iter)
{
  bool all_converged = true;
  for (auto & conv : _convergence_objects)
  {
    const auto status = conv->checkConvergence(iter);
    if (status == Convergence::MooseConvergenceStatus::DIVERGED)
      return Convergence::MooseConvergenceStatus::DIVERGED;
    else if (status == Convergence::MooseConvergenceStatus::ITERATING)
      all_converged = false;
    else
    {
      // checking in case additional status added in future
      mooseAssert(status == Convergence::MooseConvergenceStatus::CONVERGED,
                  "Status not implemented");
    }
  }

  if (all_converged)
    return Convergence::MooseConvergenceStatus::CONVERGED;
  else
    return Convergence::MooseConvergenceStatus::ITERATING;
}
