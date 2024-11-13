//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "IterationCountConvergence.h"

/**
 * Compares a post-processor to a tolerance.
 */
class PostprocessorConvergence : public IterationCountConvergence
{
public:
  static InputParameters validParams();

  PostprocessorConvergence(const InputParameters & parameters);

protected:
  virtual MooseConvergenceStatus checkConvergenceInner(unsigned int iter) override;

  /// Post-processor to use for convergence criteria
  const PostprocessorValue & _postprocessor;

  /// Tolerance to which post-processor is compared
  const Real _tol;
};
