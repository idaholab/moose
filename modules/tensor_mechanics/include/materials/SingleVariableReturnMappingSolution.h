//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef SINGLEVARIABLERETURNMAPPINGSOLUTION_H
#define SINGLEVARIABLERETURNMAPPINGSOLUTION_H

#include "InputParameters.h"
#include "ConsoleStream.h"

class SingleVariableReturnMappingSolution;

template <>
InputParameters validParams<SingleVariableReturnMappingSolution>();

/**
 * Base class that provides capability for Newton return mapping
 * iterations on a single variable
 */
class SingleVariableReturnMappingSolution
{
public:
  SingleVariableReturnMappingSolution(const InputParameters & parameters);
  virtual ~SingleVariableReturnMappingSolution() {}

  /// Functions for setting old default tolerances with legacy_return_mapping:
  void setMaxIts(unsigned int max_its) { _max_its = max_its; }
  void setRelativeTolerance(Real relative_tolerance) { _relative_tolerance = relative_tolerance; }
  void setAbsoluteTolerance(Real absolute_tolerance) { _absolute_tolerance = absolute_tolerance; }

protected:
  /**
   * Perform the return mapping iterations
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   * @param console                Console output
   */
  void returnMappingSolve(const Real effective_trial_stress,
                          Real & scalar,
                          const ConsoleStream & console);

  /**
   * Compute the maximum permissible value of the scalar.  For some models, the magnitude
   * of this may be known.
   * @param effective_trial_stress Effective trial stress
   */
  virtual Real maximumPermissibleValue(const Real effective_trial_stress) const;

  /**
   * Compute the residual for a predicted value of the scalar.  This residual should be
   * in strain increment units for all models for consistency.
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   */
  virtual Real computeResidual(const Real effective_trial_stress, const Real scalar) = 0;

  /**
   * Compute the derivative of the residual as a function of the scalar variable.  The
   * residual should be in strain increment units for all models for consistency.
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   */
  virtual Real computeDerivative(const Real effective_trial_stress, const Real scalar) = 0;

  /**
   * Compute a reference quantity to be used for checking relative convergence. This should
   * be in strain increment units for all models for consistency.
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   */
  virtual Real computeReferenceResidual(const Real effective_trial_stress, const Real scalar) = 0;

  /**
   * Finalize internal state variables for a model for a given iteration.
   * @param scalar                 Inelastic strain increment magnitude being solved for
   */
  virtual void iterationFinalize(Real /*scalar*/) {}

  /// Whether to use the legacy return mapping algorithm and compute residuals in the legacy
  /// manner.
  bool _legacy_return_mapping;

  /// Whether to check to see whether iterative solution is within admissible range, and set within that range if outside
  bool _check_range;

private:
  /// Maximum number of return mapping iterations (used only in legacy return mapping)
  unsigned int _max_its;

  /// Maximum number of return mapping iterations used in current procedure. Not settable by user.
  const unsigned int _fixed_max_its;

  /// Whether to output iteration information all the time (regardless of whether iterations converge)
  const bool _output_iteration_info;

  /// Relative convergence tolerance
  Real _relative_tolerance;

  /// Absolute convergence tolerance
  Real _absolute_tolerance;

  /// Multiplier applied to relative and absolute tolerances for acceptable convergence
  Real _acceptable_multiplier;

  /// Whether to use line searches to improve convergence
  bool _line_search;

  /// Whether to save upper and lower bounds of root for scalar, and set solution to the midpoint between
  /// those bounds if outside them
  bool _bracket_solution;

  /// Number of residuals to be stored in history
  const std::size_t _num_resids;

  /// History of residuals used to check whether progress is still being made on decreasing the residual
  std::vector<Real> _residual_history;

  /**
   * Method called from within this class to perform the actual return mappping iterations.
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   * @param iter_output            Output stream -- if null, no output is produced
   * @return Whether the solution was successful
   */
  bool
  internalSolve(const Real effective_trial_stress, Real & scalar, std::stringstream * iter_output);

  /**
   * Method called from within this class to perform the actual return mappping iterations.
   * This version uses the legacy procedure.
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   * @param iter_output            Output stream -- if null, no output is produced
   * @return Whether the solution was successful
   */
  bool internalSolveLegacy(const Real effective_trial_stress,
                           Real & scalar,
                           std::stringstream * iter_output);

  /**
   * Check to see whether the residual is within the convergence limits.
   * @param residual  Current value of the residual
   * @param reference Current value of the reference quantity
   * @return Whether the model converged
   */
  bool converged(const Real residual, const Real reference);

  /**
   * Check to see whether the residual is within acceptable convergence limits.
   * This will only return true if it has been determined that progress is no
   * longer being made and that the residual is within the acceptable limits.
   * @param residual  Current iteration count
   * @param residual  Current value of the residual
   * @param reference Current value of the reference quantity
   * @return Whether the model converged
   */
  bool convergedAcceptable(const unsigned int it, const Real residual, const Real reference);

  /**
   * Output information about convergence history of the model
   * @param iter_output            Output stream
   * @param it                     Current iteration count
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   * @param residual               Current value of the residual
   * @param reference              Current value of the reference quantity
   */
  void outputIterInfo(std::stringstream * iter_output,
                      const unsigned int it,
                      const Real effective_trial_stress,
                      const Real scalar,
                      const Real residual,
                      const Real reference_residual);

  /**
   * Check to see whether solution is within admissible range, and set it within that range
   * if it is not.
   * @param scalar                 Current value of the inelastic strain increment
   * @param scalar_increment       Incremental change in scalar from the previous iteration
   * @param scalar_old             Previous value of scalar
   * @param min_permissible_scalar Minimum permissible value of scalar
   * @param max_permissible_scalar Maximum permissible value of scalar
   * @param iter_output            Output stream
   */
  void checkPermissibleRange(Real & scalar,
                             Real & scalar_increment,
                             const Real scalar_old,
                             const Real min_permissible_scalar,
                             const Real max_permissible_scalar,
                             std::stringstream * iter_output);

  /**
   * Update the upper and lower bounds of the root for the effective inelastic strain.
   * @param scalar                 Current value of the inelastic strain increment
   * @param residual               Current value of the residual
   * @param init_resid_sign        Sign of the initial value of the residual
   * @param scalar_upper_bound     Upper bound value of scalar
   * @param scalar_lower_bound     Lower bound value of scalar
   * @param iter_output            Output stream
   */
  void updateBounds(const Real scalar,
                    const Real residual,
                    const Real init_resid_sign,
                    Real & scalar_upper_bound,
                    Real & scalar_lower_bound,
                    std::stringstream * iter_output);
};

#endif // SINGLEVARIABLERETURNMAPPINGSOLUTION_H
