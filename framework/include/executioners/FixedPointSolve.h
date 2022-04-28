//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolveObject.h"

// System includes
#include <string>

class FixedPointSolve : public SolveObject
{
public:
  FixedPointSolve(Executioner & ex);

  virtual ~FixedPointSolve() = default;

  static InputParameters validParams();

  /**
   * Iteratively solves the FEProblem.
   * @return True if solver is converged.
   */
  virtual bool solve() override;

  /// Enumeration for fixed point convergence reasons
  enum class MooseFixedPointConvergenceReason
  {
    UNSOLVED = 0,
    CONVERGED_NONLINEAR = 1,
    CONVERGED_ABS = 2,
    CONVERGED_RELATIVE = 3,
    CONVERGED_CUSTOM = 4,
    REACH_MAX_ITS = 5,
    DIVERGED_MAX_ITS = -1,
    DIVERGED_NONLINEAR = -2,
    DIVERGED_FAILED_MULTIAPP = -3
  };

  /**
   * Get the number of fixed point iterations performed
   * Because this returns the number of fixed point iterations, rather than the current
   * iteration count (which starts at 0), increment by 1.
   *
   * @return Number of fixed point iterations performed
   */
  unsigned int numFixedPointIts() const { return _fixed_point_it + 1; }

  /// Deprecated getter for the number of fixed point iterations
  unsigned int numPicardIts() const
  {
    mooseDeprecated("numPicards() is deprecated. Please use numFixedPointIts() instead.");

    return _fixed_point_it + 1;
  }

  /// Check the solver status
  MooseFixedPointConvergenceReason checkConvergence() const { return _fixed_point_status; }

  /// This function checks the _xfem_repeat_step flag set by solve.
  bool XFEMRepeatStep() const { return _xfem_repeat_step; }

  /// Clear fixed point status
  void clearFixedPointStatus() { _fixed_point_status = MooseFixedPointConvergenceReason::UNSOLVED; }

  /// Whether or not this has fixed point iterations
  bool hasFixedPointIteration() { return _has_fixed_point_its; }

  /// Set relaxation factor for the current solve as a SubApp
  void setMultiAppRelaxationFactor(Real factor) { _secondary_relaxation_factor = factor; }

  /// Set relaxation variables for the current solve as a SubApp
  void setMultiAppTransformedVariables(const std::vector<std::string> & vars)
  {
    _secondary_transformed_variables = vars;
  }

  /// Set relaxation postprocessors for the current solve as a SubApp
  virtual void setMultiAppTransformedPostprocessors(const std::vector<PostprocessorName> & pps)
  {
    _secondary_transformed_pps = pps;
  }

  /**
   * Allocate storage for the fixed point algorithm.
   * This creates the system vector of old (older, pre/post solve) variable values and the
   * array of old (older, pre/post solve) postprocessor values.
   *
   * @param primary Whether this routine is to allocate storage for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void allocateStorage(const bool primary) = 0;

  /// Whether sub-applications are automatically advanced no matter what happens during their solves
  bool autoAdvance() const;

  /// Mark the current solve as failed due to external conditions
  void failStep() { _fail_step = true; }

protected:
  /**
   * Saves the current values of the variables, and update the old(er) vectors.
   *
   * @param primary Whether this routine is to save the variables for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void saveVariableValues(const bool primary) = 0;

  /**
   * Saves the current values of the postprocessors, and update the old(er) vectors.
   *
   * @param primary Whether this routine is to save the variables for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void savePostprocessorValues(const bool primary) = 0;

  /**
   * Use the fixed point algorithm transform instead of simply using the Picard update
   * This routine can be used to alternate Picard iterations and fixed point algorithm
   * updates based on the values of the variables before and after a solve / a Picard iteration.
   *
   * @param primary Whether this routine is used for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual bool useFixedPointAlgorithmUpdateInsteadOfPicard(const bool primary) = 0;

  /**
   * Perform one fixed point iteration or a full solve.
   *
   * @param begin_norm       Residual norm after timestep_begin execution
   * @param end_norm         Residual norm after timestep_end execution
   * @param transformed_dofs DoFs targetted by the fixed point algorithm
   *
   * @return True if both nonlinear solve and the execution of multiapps are successful.
   *
   * Note: this function also set _xfem_repeat_step flag for XFEM. It tracks _xfem_update_count
   * state.
   * FIXME: The proper design will be to let XFEM use Picard iteration to control the execution.
   */
  virtual bool
  solveStep(Real & begin_norm, Real & end_norm, const std::set<dof_id_type> & transformed_dofs);

  /// Save both the variable and postprocessor values
  virtual void saveAllValues(const bool primary);

  /**
   * Use the fixed point algorithm to transform the postprocessors.
   * If this routine is not called, the next value of the postprocessors will just be from
   * the unrelaxed Picard fixed point algorithm.
   *
   * @param primary Whether this routine is to save the variables for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void transformPostprocessors(const bool primary) = 0;

  /**
   * Use the fixed point algorithm to transform the variables.
   * If this routine is not called, the next value of the variables will just be from
   * the unrelaxed Picard fixed point algorithm.
   *
   * @param transformed_dofs The dofs that will be affected by the algorithm
   * @param primary Whether this routine is to save the variables for the primary transformed
   *                quantities (as main app) or the secondary ones (as a subapp)
   */
  virtual void transformVariables(const std::set<dof_id_type> & transformed_dofs,
                                  const bool primary) = 0;

  /// Print the convergence history of the coupling, at every fixed point iteration
  virtual void printFixedPointConvergenceHistory() = 0;

  /// Computes and prints the user-specified postprocessor assessing convergence
  void computeCustomConvergencePostprocessor();

  /// Examine the various convergence metrics
  bool examineFixedPointConvergence(bool & converged);

  /// Print information about the fixed point convergence
  void printFixedPointConvergenceReason();

  /// Minimum fixed point iterations
  unsigned int _min_fixed_point_its;
  /// Maximum fixed point iterations
  unsigned int _max_fixed_point_its;
  /// Whether or not we activate fixed point iteration
  bool _has_fixed_point_its; // TODO: make const once picard parameters are removed
  /// Whether or not to treat reaching maximum number of fixed point iteration as converged
  bool _accept_max_it; // TODO: make const once picard parameters are removed
  /// Whether or not to use residual norm to check the fixed point convergence
  bool _has_fixed_point_norm; // TODO: make const once picard parameters are removed
  /// Relative tolerance on residual norm
  Real _fixed_point_rel_tol; // TODO: make const once picard parameters are removed
  /// Absolute tolerance on residual norm
  Real _fixed_point_abs_tol; // TODO: make const once picard parameters are removed
  /// Whether or not we force evaluation of residual norms even without multiapps
  bool _fixed_point_force_norms; // TODO: make const once picard parameters are removed

  /// Postprocessor value for user-defined fixed point convergence check
  const PostprocessorValue *
      _fixed_point_custom_pp; // FIXME Make const and private once picard_custom_pp is gone

  /// Relaxation factor for fixed point Iteration
  const Real _relax_factor;
  /// The variables (transferred or not) that are going to be relaxed
  std::vector<std::string> _transformed_vars; // TODO: make const once relaxed_variables is removed
  /// The postprocessors (transferred or not) that are going to be relaxed
  const std::vector<PostprocessorName> _transformed_pps;
  /// Previous values of the relaxed postprocessors
  std::vector<std::vector<PostprocessorValue>> _transformed_pps_values;

  /// Relaxation factor outside of fixed point iteration (used as a subapp)
  Real _secondary_relaxation_factor;
  /// Variables to be relaxed outside of fixed point iteration (used as a subapp)
  std::vector<std::string> _secondary_transformed_variables;
  /// Postprocessors to be relaxed outside of fixed point iteration (used as a subapp)
  std::vector<PostprocessorName> _secondary_transformed_pps;
  /// Previous values of the postprocessors relaxed outside of the fixed point iteration (used as a subapp)
  std::vector<std::vector<PostprocessorValue>> _secondary_transformed_pps_values;

  ///@{ Variables used by the fixed point iteration
  /// fixed point iteration counter
  unsigned int _fixed_point_it;
  /// fixed point iteration counter for the main app
  unsigned int _main_fixed_point_it;
  /// Initial residual norm
  Real _fixed_point_initial_norm;
  /// Full history of residual norm after evaluation of timestep_begin
  std::vector<Real> _fixed_point_timestep_begin_norm;
  /// Full history of residual norm after evaluation of timestep_end
  std::vector<Real> _fixed_point_timestep_end_norm;
  /// Status of fixed point solve
  MooseFixedPointConvergenceReason _fixed_point_status;
  ///@}
private:
  /// Relative tolerance on postprocessor value
  const Real _custom_rel_tol;
  /// Absolute tolerance on postprocessor value
  const Real _custom_abs_tol;
  /// Old value of the custom convergence check postprocessor
  Real _pp_old;
  /// Current value of the custom convergence check postprocessor
  Real _pp_new;
  /// Scaling of custom convergence check postprocessor (its initial value)
  Real _pp_scaling;
  /// Convergence history of the custom convergence check postprocessor
  std::ostringstream _pp_history;

  /// Maximum number of xfem updates per step
  const unsigned int _max_xfem_update;
  /// Controls whether xfem should update the mesh at the beginning of the time step
  const bool _update_xfem_at_timestep_begin;

  /// Counter for number of xfem updates that have been performed in the current step
  unsigned int _xfem_update_count;
  /// Whether step should be repeated due to xfem modifying the mesh
  bool _xfem_repeat_step;

  /// Time of previous fixed point solve as a subapp
  Real _old_entering_time;

  /// force the current step to fail, triggering are repeat with a cut dt
  bool _fail_step;

  /// Whether the user has set the auto_advance parameter for handling advancement of
  /// sub-applications in multi-app contexts
  const bool _auto_advance_set_by_user;

  /// The value of auto_advance set by the user for handling advancement of sub-applications in
  /// multi-app contexts
  const bool _auto_advance_user_value;
};
