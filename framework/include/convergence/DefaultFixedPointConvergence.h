//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DefaultConvergenceBase.h"

class FixedPointSolve;

/**
 * Default fixed point convergence criteria.
 */
class DefaultFixedPointConvergence : public DefaultConvergenceBase
{
public:
  static InputParameters validParams();

  DefaultFixedPointConvergence(const InputParameters & parameters);

  virtual void initialize() override;

  virtual MooseConvergenceStatus checkConvergence(unsigned int iter) override;

protected:
  /// Computes and prints the user-specified postprocessor assessing convergence
  void computeCustomConvergencePostprocessor(unsigned int iter);

  /// Whether or not to treat reaching maximum number of fixed point iteration as converged
  const bool _accept_max_it;
  /// Relative tolerance on residual norm
  const Real _fixed_point_rel_tol;
  /// Absolute tolerance on residual norm
  const Real _fixed_point_abs_tol;
  /// Relative tolerance on postprocessor value
  const Real _custom_pp_rel_tol;
  /// Absolute tolerance on postprocessor value
  const Real _custom_pp_abs_tol;

  /// Postprocessor value for user-defined fixed point convergence check
  const PostprocessorValue * const _fixed_point_custom_pp;
  /// Old value of the custom convergence check postprocessor
  Real _pp_old;
  /// Current value of the custom convergence check postprocessor
  Real _pp_new;
  /// Scaling of custom convergence check postprocessor (its initial value)
  Real _pp_scaling;
  /// Convergence history of the custom convergence check postprocessor
  std::ostringstream _pp_history;

  /// Fixed point solve
  FixedPointSolve & _fp_solve;
};
