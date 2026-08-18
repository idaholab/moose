//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Control.h"

#include <optional>

class Function;

/**
 * Base class for controls that adjust a single scalar parameter (held in a postprocessor) once per
 * fixed-point iteration so that a sub-application output postprocessor matches a target function of
 * time. It factors the shared plumbing (parameter/output postprocessors, target function, and
 * optional publishing of the converged parameter) and the guarded secant update; derived classes
 * implement computeUpdate() with their specific iteration scheme. The outer iteration, convergence,
 * and time-step cutting are owned by the executioner and the Convergence system.
 */
class InversionControlBase : public Control
{
public:
  static InputParameters validParams();

  InversionControlBase(const InputParameters & parameters);

  /**
   * Template method: reads the current (parameter, output) sample, delegates the scheme-specific
   * decision to computeUpdate(), and applies it (publish, residual, next parameter). Derived
   * classes implement computeUpdate(), not this.
   */
  virtual void execute() override final;

protected:
  /// One iteration's update, produced by the derived scheme and applied by execute().
  struct IterationUpdate
  {
    /// Parameter guess to write for the next solve
    Real next_parameter;
    /// Residual to report for the current sample (compared against 1 by the Convergence object)
    Real residual;
    /// Whether the current (already-evaluated) parameter is the solution of record to publish
    bool publish;
  };

  /**
   * Compute this iteration's update from the current sample: the 1-based sweep iteration \c it and
   * the (parameter, output) pair (\c p_used, \c y) that produced it, against target \c y_target.
   */
  virtual IterationUpdate computeUpdate(unsigned int it, Real p_used, Real y, Real y_target) = 0;

  /// 1-based fixed-point sweep iteration (1 on the first iteration of each sweep, including restep)
  unsigned int sweepIteration() const;

  /// Target output value at the current time
  Real targetValue() const;

  /// Publish a parameter value to the optional converged-parameter postprocessor (no-op if unset)
  void publishConvergedParameter(Real p);

  /// Write the next parameter guess to the parameter postprocessor
  void setParameter(Real p);

  /**
   * One guarded linear-model root update from two (parameter, output) samples -- a secant step
   * shared by the secant and finite-difference Newton schemes:
   * returns \c p_b - (y_b - y_target) * (p_b - p_a) / (y_b - y_a). If the denominator or slope is
   * too small to divide by, emits a warning and returns \c p_b unchanged.
   */
  Real linearRootUpdate(Real p_a, Real y_a, Real p_b, Real y_b, Real y_target);

  /**
   * Normalized convergence residual for output value \c y:
   * |y - targetValue()| / max(absolute_tolerance, relative_tolerance*|targetValue()|).
   * A value <= 1 means the absolute OR relative criterion is satisfied. The denominator is always
   * positive because absolute_tolerance is range-checked > 0, so no divide guard is needed.
   */
  Real normalizedResidual(Real y) const;

  /// Write a value to the residual postprocessor that the Convergence object compares against 1.
  void setResidual(Real value);

  /// Sub-app output value transferred back to the main app (compared against the target)
  const PostprocessorValue & _output;
  /// Parameter value consumed by the most recent sub-app solve (read in place)
  const PostprocessorValue & _param;
  /// Name of the parameter postprocessor to update with the next guess
  const PostprocessorName _param_name;
  /// Optional postprocessor to publish the parameter value that produced the current output
  const std::optional<PostprocessorName> _converged_param_name;
  /// Target output as a function of time
  const Function & _target_function;
  /// Name of the postprocessor the control writes with the normalized convergence residual
  const PostprocessorName _residual_name;
  /// Absolute tolerance on |output - target|
  const Real _abs_tol;
  /// Relative tolerance on |output - target|, relative to |target|
  const Real _rel_tol;
};
