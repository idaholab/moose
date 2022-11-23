//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "DualRealOps.h"
#include "InputParameters.h"
#include "MooseTypes.h"

class ConsoleStream;

/**
 * Base class that provides capability for Newton generalized (anisotropic) return mapping
 * iterations on a single variable
 */
template <bool is_ad>
class GeneralizedReturnMappingSolutionTempl
{
public:
  static InputParameters validParams();

  GeneralizedReturnMappingSolutionTempl(const InputParameters & parameters);
  virtual ~GeneralizedReturnMappingSolutionTempl() {}

protected:
  /**
   * Perform the return mapping iterations
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   * @param console                Console output
   */
  void returnMappingSolve(const GenericDenseVector<is_ad> & effective_trial_stress,
                          const GenericDenseVector<is_ad> & stress_new,
                          GenericReal<is_ad> & scalar,
                          const ConsoleStream & console);
  /**
   * Compute the minimum permissible value of the scalar.  For some models, the magnitude
   * of this may be known.
   * @param effective_trial_stress Effective trial stress
   */
  virtual GenericReal<is_ad>
  minimumPermissibleValue(const GenericDenseVector<is_ad> & effective_trial_stress) const;

  /**
   * Compute the maximum permissible value of the scalar.  For some models, the magnitude
   * of this may be known.
   * @param effective_trial_stress Effective trial stress
   */
  virtual GenericReal<is_ad>
  maximumPermissibleValue(const GenericDenseVector<is_ad> & effective_trial_stress) const;

  /**
   * Compute an initial guess for the value of the scalar. For some cases, an
   * intellegent starting point can provide enhanced robustness in the Newton
   * iterations. This is also an opportunity for classes that derive from this
   * to perform initialization tasks.
   * @param effective_trial_stress Effective trial stress
   */
  virtual GenericReal<is_ad>
  initialGuess(const GenericDenseVector<is_ad> & /*effective_trial_stress*/)
  {
    return 0.0;
  }

  /**
   * Compute the residual for a predicted value of the scalar.  This residual should be
   * in strain increment units for all models for consistency.
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   */
  virtual GenericReal<is_ad>
  computeResidual(const GenericDenseVector<is_ad> & effective_trial_stress,
                  const GenericDenseVector<is_ad> & stress_new,
                  const GenericReal<is_ad> & delta_gamma) = 0;

  /**
   * Compute a reference quantity to be used for checking relative convergence. This should
   * be in strain increment units for all models for consistency.
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   */
  virtual Real
  computeReferenceResidual(const GenericDenseVector<is_ad> & effective_trial_stress,
                           const GenericDenseVector<is_ad> & stress_new,
                           const GenericReal<is_ad> & residual,
                           const GenericReal<is_ad> & scalar_effective_inelastic_strain) = 0;

  virtual GenericReal<is_ad>
  computeDerivative(const GenericDenseVector<is_ad> & effective_trial_stress,
                    const GenericDenseVector<is_ad> & stress_new,
                    const GenericReal<is_ad> & scalar) = 0;
  /**
   * Finalize internal state variables for a model for a given iteration.
   * @param scalar                 Inelastic strain increment magnitude being solved for
   */
  virtual void iterationFinalize(GenericReal<is_ad> /*scalar*/) {}

  /**
   * Output summary information for the convergence history of the model
   * @param iter_output            Output stream
   * @param total_it               Total iteration count
   */
  virtual void outputIterationSummary(std::stringstream * iter_output, const unsigned int total_it);

  /// Whether to check to see whether iterative solution is within admissible range, and set within that range if outside
  bool _check_range;

  /// Whether to use line searches to improve convergence
  bool _line_search;

  /// Whether to save upper and lower bounds of root for scalar, and set solution to the midpoint between
  /// those bounds if outside them
  bool _bracket_solution;

  /**
   * Output information for a single iteration step to build the convergence history of the model
   * @param iter_output            Output stream
   * @param it                     Current iteration count
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   * @param residual               Current value of the residual
   * @param reference              Current value of the reference quantity
   */
  virtual void outputIterationStep(std::stringstream * iter_output,
                                   const GenericDenseVector<is_ad> & effective_trial_stress,
                                   const GenericReal<is_ad> & scalar,
                                   const GenericReal<is_ad> reference_residual);

  /**
   * Check to see whether the residual is within the convergence limits.
   * @param residual  Current value of the residual
   * @param reference Current value of the reference quantity
   * @return Whether the model converged
   */
  bool converged(const GenericReal<is_ad> & residual, const Real & reference);

private:
  enum class InternalSolveOutput
  {
    NEVER,
    ON_ERROR,
    ALWAYS
  } _internal_solve_output_on;

  enum class SolveState
  {
    SUCCESS,
    NAN_INF,
    EXCEEDED_ITERATIONS
  };

  /// Maximum number of return mapping iterations. This exists only to avoid an infinite loop, and is
  /// is intended to be a large number that is not settable by the user.
  const unsigned int _max_its;

  /// Whether to output iteration information all the time (regardless of whether iterations converge)
  const bool _internal_solve_full_iteration_history;

  /// Relative convergence tolerance
  Real _relative_tolerance;

  /// Absolute convergence tolerance
  Real _absolute_tolerance;

  /// Multiplier applied to relative and absolute tolerances for acceptable convergence
  Real _acceptable_multiplier;

  /// Number of residuals to be stored in history
  const std::size_t _num_resids;

  /// History of residuals used to check whether progress is still being made on decreasing the residual
  std::vector<Real> _residual_history;

  /// iteration number
  unsigned int _iteration;

  ///@{ Residual values, kept as members to retain solver state for summary outputting
  GenericReal<is_ad> _initial_residual;
  GenericReal<is_ad> _residual;
  ///@}

  /// MOOSE input name of the object performing the solve
  const std::string _svrms_name;

  /**
   * Method called from within this class to perform the actual return mappping iterations.
   * @param effective_trial_stress Effective trial stress
   * @param scalar                 Inelastic strain increment magnitude being solved for
   * @param iter_output            Output stream -- if null, no output is produced
   * @return Whether the solution was successful
   */
  SolveState internalSolve(const GenericDenseVector<is_ad> & effective_trial_stress,
                           const GenericDenseVector<is_ad> & stress_new,
                           GenericReal<is_ad> & scalar,
                           std::stringstream * iter_output = nullptr);

  /**
   * Check to see whether the residual is within acceptable convergence limits.
   * This will only return true if it has been determined that progress is no
   * longer being made and that the residual is within the acceptable limits.
   * @param residual  Current iteration count
   * @param residual  Current value of the residual
   * @param reference Current value of the reference quantity
   * @return Whether the model converged
   */
  bool convergedAcceptable(const unsigned int it, const Real & reference);

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
  void checkPermissibleRange(GenericReal<is_ad> & scalar,
                             GenericReal<is_ad> & scalar_increment,
                             const GenericReal<is_ad> & scalar_old,
                             const GenericReal<is_ad> min_permissible_scalar,
                             const GenericReal<is_ad> max_permissible_scalar,
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
  void updateBounds(const GenericReal<is_ad> & scalar,
                    const GenericReal<is_ad> & residual,
                    const Real init_resid_sign,
                    GenericReal<is_ad> & scalar_upper_bound,
                    GenericReal<is_ad> & scalar_lower_bound,
                    std::stringstream * iter_output);
};

typedef GeneralizedReturnMappingSolutionTempl<false> GeneralizedReturnMappingSolution;
typedef GeneralizedReturnMappingSolutionTempl<true> ADGeneralizedReturnMappingSolution;
