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

class IterativeMultiAppSolve;

template <>
InputParameters validParams<IterativeMultiAppSolve>();

class IterativeMultiAppSolve : public SolveObject
{
public:
  IterativeMultiAppSolve(Executioner * ex);

  virtual ~IterativeMultiAppSolve() = default;

  static InputParameters validParams();

  /**
   * Iteratively solves the FEProblem.
   * @return True if solver is converged.
   */
  virtual bool solve() override;

  /// Enumeration for coupling convergence reasons
  enum class MooseCouplingConvergenceReason
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
   * Get the number of coupling iterations performed
   * Because this returns the number of coupling iterations, rather than the current
   * iteration count (which starts at 0), increment by 1.
   *
   * @return Number of coupling iterations performed
   */
  unsigned int numCouplingIts() const { return _coupling_it + 1; }

  /// Deprecated getter for the number of coupling iterations
  unsigned int numPicardIts() const
  {
    mooseDeprecated("numPicards() is deprecated. Please use numCouplingIts() instead.");

    return _coupling_it + 1;
  }

  /// Check the solver status
  MooseCouplingConvergenceReason checkConvergence() const { return _coupling_status; }

  /// This function checks the _xfem_repeat_step flag set by solve.
  bool XFEMRepeatStep() const { return _xfem_repeat_step; }

  /// Clear coupling status
  void clearCouplingStatus() { _coupling_status = MooseCouplingConvergenceReason::UNSOLVED; }

  /// Whether or not this has coupling iterations
  bool hasCouplingIteration() { return _has_coupling_its; }

  /// Set relaxation factor for the current solve as a SubApp
  void setMultiAppRelaxationFactor(Real factor) { _secondary_relaxation_factor = factor; }

  /// Set relaxation variables for the current solve as a SubApp
  void setMultiAppRelaxationVariables(const std::vector<std::string> & vars)
  {
    _secondary_transformed_variables = vars;
  }

  /// Set relaxation postprocessors for the current solve as a SubApp
  virtual void setMultiAppRelaxationPostprocessors(const std::vector<std::string> & pps)
  {
    _secondary_transformed_pps = pps;
    _old_secondary_transformed_pps_values.resize(pps.size());
  }

  /// Save the variable and postprocessor values as a SubApp
  virtual void savePreviousValuesAsSubApp() = 0;

  /// Whether to use the coupling algorithm (relaxed Picard, Secant, ...) instead of Picard
  virtual bool useCouplingUpdateAlgorithm() = 0;

  /**
   * Perform one coupling iteration or a full solve.
   *
   * @param begin_norm     Residual norm after timestep_begin execution
   * @param end_norm       Residual norm after timestep_end execution
   * @param target_dofs    DoFs targetted by the coupling algorithm
   *
   * @return True if both nonlinear solve and the execution of multiapps are successful.
   *
   * Note: this function also set _xfem_repeat_step flag for XFEM. It tracks _xfem_update_count
   * state.
   * FIXME: The proper design will be to let XFEM use Picard iteration to control the execution.
   */
  virtual bool solveStep(Real & begin_norm,
                         Real & end_norm,
                         const std::set<dof_id_type> & transformed_dofs);

  /// Save the previous values for the variables
  virtual void savePreviousValuesAsMainApp() = 0;

  /// Compute the new value of the coupling postprocessors based on the coupling algorithm selected
  virtual void updatePostprocessorsAsMainApp() = 0;

  /// Compute the new value variable values based on the coupling algorithm selected
  virtual void updateVariablesAsMainApp(const std::set<dof_id_type> & transformed_dofs) = 0;

  /// Update variables and postprocessors as a SubApp
  virtual void updateAsSubApp(const std::set<dof_id_type> & secondary_transformed_dofs) = 0;

  /// Print the convergence history of the coupling, at every coupling iteration
  virtual void printCouplingConvergenceHistory() = 0;

  /// Computes and prints the user-specified postprocessor assessing convergence
  void computeCustomConvergencePostprocessor();

  /// Examine the various convergence metrics
  bool examineCouplingConvergence(bool & converged);

  /// Print information about the coupling convergence
  void printCouplingConvergenceReason();

  /// Whether sub-applications are automatically advanced no matter what happens during their solves
  bool autoAdvance() const;

  /// Mark the current solve as failed due to external conditions
  void failStep() { _fail_step = true; }

protected:
  /// Minimum coupling iterations
  unsigned int _coupling_min_its;
  /// Maximum coupling iterations
  unsigned int _coupling_max_its;
  /// Whether or not we activate coupling iteration
  bool _has_coupling_its; // TODO: make const once picard parameters are removed
  /// Whether or not to treat reaching maximum number of coupling iteration as converged
  bool _accept_max_it; // TODO: make const once picard parameters are removed
  /// Whether or not to use residual norm to check the coupling convergence
  bool _has_coupling_norm; // TODO: make const once picard parameters are removed
  /// Relative tolerance on residual norm
  Real _coupling_rel_tol; // TODO: make const once picard parameters are removed
  /// Absolute tolerance on residual norm
  Real _coupling_abs_tol; // TODO: make const once picard parameters are removed
  /// Whether or not we force evaluation of residual norms even without multiapps
  bool _coupling_force_norms; // TODO: make const once picard parameters are removed

  /// Postprocessor value for user-defined coupling convergence check
  const PostprocessorValue * _coupling_custom_pp; //FIXME Make const once picard_custom_pp is gone
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

  /// Relaxation factor for coupling Iteration
  const Real _relax_factor;
  /// The variables (transferred or not) that are going to be relaxed
  std::vector<std::string> _transformed_vars; // TODO: make const once relaxed_variables is removed
  /// The postprocessors (transferred or not) that are going to be relaxed
  const std::vector<std::string> _transformed_pps;
  /// Previous values of the relaxed postprocessors
  std::vector<PostprocessorValue> _old_transformed_pps_values;

  /// Relaxation factor outside of coupling iteration (used as a subapp)
  Real _secondary_relaxation_factor;
  /// Variables to be relaxed outside of coupling iteration (used as a subapp)
  std::vector<std::string> _secondary_transformed_variables;
  /// Postprocessors to be relaxed outside of coupling iteration (used as a subapp)
  std::vector<std::string> _secondary_transformed_pps;
  /// Previous values of the postprocessors relaxed outside of the coupling iteration (used as a subapp)
  std::vector<PostprocessorValue> _old_secondary_transformed_pps_values;
  /// Whether previous self relaxed postprocessors have been stored
  bool _has_old_pp_values;

  /// Maximum number of xfem updates per step
  const unsigned int _max_xfem_update;
  /// Controls whether xfem should update the mesh at the beginning of the time step
  const bool _update_xfem_at_timestep_begin;

  /// Timer for coupling iteration
  const PerfID _coupling_timer;

  ///@{ Variables used by the coupling iteration
  /// coupling iteration counter
  unsigned int _coupling_it;
  /// Initial residual norm
  Real _coupling_initial_norm;
  /// Full history of residual norm after evaluation of timestep_begin
  std::vector<Real> _coupling_timestep_begin_norm;
  /// Full history of residual norm after evaluation of timestep_end
  std::vector<Real> _coupling_timestep_end_norm;
  /// Status of coupling solve
  MooseCouplingConvergenceReason _coupling_status;
  ///@}

  /// Counter for number of xfem updates that have been performed in the current step
  unsigned int _xfem_update_count;
  /// Whether step should be repeated due to xfem modifying the mesh
  bool _xfem_repeat_step;

  /// Time of previous coupling solve as a subapp
  Real _old_entering_time;

  const std::string _solve_message;

  /// force the current step to fail, triggering are repeat with a cut dt
  bool _fail_step;

private:

  /// Whether the user has set the auto_advance parameter for handling advancement of
  /// sub-applications in multi-app contexts
  const bool _auto_advance_set_by_user;

  /// The value of auto_advance set by the user for handling advancement of sub-applications in
  /// multi-app contexts
  const bool _auto_advance_user_value;
};
