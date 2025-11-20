//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IterationCountConvergence.h"

class THMProblem;

/**
 * Assesses convergence of all Component objects in a simulation.
 */
class ComponentsConvergence : public IterationCountConvergence
{
public:
  static InputParameters validParams();

  ComponentsConvergence(const InputParameters & parameters);

  virtual void initialSetup() override;

protected:
  virtual MooseConvergenceStatus checkConvergenceInner(unsigned int iter) override;

  /// THM problem
  const THMProblem * const _thm_problem;

  /// Convergence objects for all Components that provide one
  std::vector<Convergence *> _convergence_objects;
};
