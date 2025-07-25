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

class FEProblemBase;
class FixedPointSolve;

/**
 * Default fixed point convergence criteria.
 */
class DefaultMultiAppFixedPointConvergence : public DefaultConvergenceBase
{
public:
  static InputParameters validParams();

  DefaultMultiAppFixedPointConvergence(const InputParameters & parameters);

  virtual void checkIterationType(IterationType it_type) const override;
  virtual void initialize() override;
  virtual void preExecute() override;

  virtual MooseConvergenceStatus checkConvergence(unsigned int iter) override;

protected:
  /// Outputs residual norm to console
  void outputResidualNorm(const std::string & execute_on_str, Real old_norm, Real new_norm) const;

  /// Computes and prints the user-specified postprocessor assessing convergence
  void computeCustomConvergencePostprocessor(unsigned int iter);

  /// Minimum fixed point iterations
  unsigned int _min_fixed_point_its;
  /// Maximum fixed point iterations
  unsigned int _max_fixed_point_its;
  /// Whether or not to use residual norm to check the fixed point convergence
  const bool _has_fixed_point_norm;
  /// Whether or not we force evaluation of residual norms even without multiapps
  const bool _fixed_point_force_norms;
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

  /// Initial residual norm
  Real _fixed_point_initial_norm;
  /// Full history of residual norm after evaluation of timestep_begin
  std::vector<Real> _fixed_point_timestep_begin_norm;
  /// Full history of residual norm after evaluation of timestep_end
  std::vector<Real> _fixed_point_timestep_end_norm;

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

  /// FE Problem
  FEProblemBase & _fe_problem;
  /// Fixed point solve
  FixedPointSolve & _fp_solve;
};
