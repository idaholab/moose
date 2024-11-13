//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Convergence.h"

/**
 * Checks convergence based on the iteration count.
 */
class IterationCountConvergence : public Convergence
{
public:
  static InputParameters validParams();

  IterationCountConvergence(const InputParameters & parameters);

  virtual MooseConvergenceStatus checkConvergence(unsigned int iter) override final;

protected:
  /**
   * Inner check of convergence.
   *
   * Derived classes are responsible for overriding this instead of \c checkConvergence().
   *
   * @param[in] iter   Iteration index
   */
  virtual MooseConvergenceStatus checkConvergenceInner(unsigned int iter);

  /// Minimum number of iterations
  const unsigned int _min_iterations;

  /// Maximum number of iterations
  const unsigned int _max_iterations;

  /// Whether to converge at the maximum number of iterations
  const bool _converge_at_max_iterations;
};
