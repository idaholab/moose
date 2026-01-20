//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Convergence.h"

/**
 * Compares a post-processor to a tolerance for steady-state checks.
 */
class PostprocessorSteadyStateConvergence : public Convergence
{
public:
  static InputParameters validParams();

  PostprocessorSteadyStateConvergence(const InputParameters & parameters);

  virtual MooseConvergenceStatus checkConvergence(unsigned int iter) override;

protected:
  /// Post-processor to use for convergence criteria
  const PostprocessorValue & _postprocessor;

  /// Tolerance to which post-processor is compared
  const Real _tol;
};
