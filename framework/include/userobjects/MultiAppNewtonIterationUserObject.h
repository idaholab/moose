//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralUserObject.h"

class MultiApp;

/**
 * Drives a TransientMultiApp through a Newton iteration to solve an inverse problem:
 * find the parameter p such that output(p) = f_target(t).
 */
class MultiAppNewtonIterationUserObject : public GeneralUserObject
{
public:
  static InputParameters validParams();

  MultiAppNewtonIterationUserObject(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}

protected:
  /**
   * Set the parameter postprocessor on global sub-app 'index' of the given MultiApp.
   * No-op on ranks that do not own that sub-app (hasLocalApp check is applied internally).
   */
  void setSubAppParam(std::shared_ptr<MultiApp> & app, unsigned int index, Real value) const;

  /**
   * Read the output postprocessor from global sub-app 'index' of the given MultiApp.  The
   * owning sub-app's root rank contributes the value, which is then reduced across all MPI
   * ranks so every processor holds the same result.
   */
  Real getSubAppOutput(std::shared_ptr<MultiApp> & app, unsigned int index) const;

  /**
   * Perform one Newton trial: evaluate the sub-app output at p (y1) and at p + delta (y2).
   * In sequential mode the base (p) solve is performed last, so on return sub-app 0 holds the
   * p-solution.  Returns true if all underlying solves converged.
   */
  bool solveTrial(std::shared_ptr<MultiApp> & app, Real p, Real & y1, Real & y2) const;

  /**
   * Solve every sub-app of the MultiApp at parameter p from the start-of-timestep state.
   * Used for the final advance when the sub-app(s) are not already in the desired p-state.
   * Returns true if all underlying solves converged.
   */
  bool finalSolveAtP(std::shared_ptr<MultiApp> & app, Real p) const;

private:
  /// Auto-generated name of the internally-created MultiApp
  const MultiAppName _multiapp_name;

  /// Name of the Receiver postprocessor in the sub-apps that accepts the parameter
  const PostprocessorName _param_pp;
  /// Name of the output postprocessor in the sub-apps to compare against the target
  const PostprocessorName _output_pp;

  /// Target output value as a function of time
  const Function & _target_function;

  /// Fixed perturbation delta_p used to estimate df/dp numerically
  const Real _delta_param;
  /// Absolute convergence tolerance on the output residual |y1 - y_target|
  const Real _abs_tol;
  /// Relative convergence tolerance: converged when |y1 - y_target| < rel_tol * |y_target|
  const Real _rel_tol;
  /// Maximum number of Newton iterations per time step
  const unsigned int _max_its;

  /// Current converged parameter value; persists across time steps (Restartable)
  Real & _param_value;

  /// _param_value as it was at the start of the time step currently being solved; used to reset
  /// _param_value when a time step is repeated (e.g. --test-restep or a cut time step)
  Real & _param_value_step_begin;
  /// Time step index of the most recent execute(); detecting the same index again means the step
  /// is being repeated, so the internally-created MultiApp must be restored rather than re-backed
  /// up (the framework does not roll it back because it is registered EXEC_NONE)
  int & _executed_t_step;

  /// Optional main-app Receiver postprocessor name to publish the converged parameter
  const PostprocessorName _param_output_pp;

  /// If true, evaluate the p and p+delta solves concurrently using two sub-apps (more memory,
  /// faster on >= 2 MPI ranks); otherwise solve a single sub-app twice sequentially.
  const bool _concurrent;

  /// Whether to accept the best estimate (true) or cut the time step (false) when the maximum
  /// number of Newton iterations is reached without convergence
  const bool _accept_on_max_iteration;
};
