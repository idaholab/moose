//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef PICARDSOLVE_H
#define PICARDSOLVE_H

#include "SolveObject.h"

// System includes
#include <string>

class PicardSolve;

template <>
InputParameters validParams<PicardSolve>();

class PicardSolve : public SolveObject
{
public:
  PicardSolve(Executioner * ex);

  /**
   * Picard solve the FEProblem.
   * @return True if solver is converged.
   */
  virtual bool solve() override;

  /// Enumeration for Picard convergence reasons
  enum class MoosePicardConvergenceReason
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
   * Get the number of Picard iterations performed
   * Because this returns the number of Picard iterations, rather than the current
   * iteration count (which starts at 0), increment by 1.
   *
   * @return Number of Picard iterations performed
   */
  unsigned int numPicardIts() const { return _picard_it + 1; }

  /// Check the solver status
  MoosePicardConvergenceReason checkConvergence() const { return _picard_status; }

  /// This function checks the _xfem_repeat_step flag set by solve.
  bool XFEMRepeatStep() const { return _xfem_repeat_step; }

  /// Clear Picard status
  void clearPicardStatus() { _picard_status = MoosePicardConvergenceReason::UNSOLVED; }

  /// Whether or not this has Picard iterations
  bool hasPicardIteration() { return _has_picard_its; }

  /// Set relaxation factor for the current solve as a MultiApp
  void setMultiAppRelaxationFactor(Real factor) { _picard_self_relaxation_factor = factor; }

  /// Set relaxation variables for the current solve as a MultiApp
  void setMultiAppRelaxationVariables(const std::vector<std::string> & vars)
  {
    _picard_self_relaxed_variables = vars;
  }

protected:
  /**
   * Perform one Picard iteration or a full solve.
   *
   * @param begin_norm_old Residual norm after timestep_begin execution of previous Picard
   * iteration
   * @param begin_norm     Residual norm after timestep_begin execution
   * @param end_norm_old   Residual norm after timestep_end execution of previous Picard iteration
   * @param end_norm       Residual norm after timestep_end execution
   * @param relax          Whether or not we do relaxation in this iteration
   * @param relaxed_dofs   DoFs to be relaxed
   *
   * @return True if both nonlinear solve and the execution of multiapps are successful.
   *
   * Note: this function also set _xfem_repeat_step flag for XFEM. It tracks _xfem_update_count
   * state.
   * FIXME: The proper design will be to let XFEM use Picard iteration to control the execution.
   */
  bool solveStep(Real begin_norm_old,
                 Real & begin_norm,
                 Real end_norm_old,
                 Real & end_norm,
                 bool relax,
                 const std::set<dof_id_type> & relaxed_dofs);

  /// Maximum Picard iterations
  unsigned int _picard_max_its;
  /// Whether or not we activate Picard iteration
  bool _has_picard_its;
  /// Whether or not to treat reaching maximum number of Picard iteration as converged
  bool _accept_max_it;
  /// Whether or not to use residual norm to check the Picard convergence
  bool _has_picard_norm;
  /// Relative tolerance on residual norm
  Real _picard_rel_tol;
  /// Absolute tolerance on residual norm
  Real _picard_abs_tol;
  /// Whether or not we force evaluation of residual norms even without multiapps
  bool _picard_force_norms;
  /// Relaxation factor for Picard Iteration
  Real _relax_factor;
  /// The transferred variables that are going to be relaxed
  std::vector<std::string> _relaxed_vars;

  /// Relaxation factor outside of Picard iteration (used as a subapp)
  Real _picard_self_relaxation_factor;
  /// Variables to be relaxed outside of Picard iteration (used as a subapp)
  std::vector<std::string> _picard_self_relaxed_variables;

  /// Maximum number of xfem updates per step
  unsigned int _max_xfem_update;
  /// Controls whether xfem should update the mesh at the beginning of the time step
  bool _update_xfem_at_timestep_begin;

private:
  /// Timer for Picard iteration
  const PerfID _picard_timer;

  ///@{ Variables used by the Picard iteration
  /// Picard iteration counter
  unsigned int _picard_it;
  /// Initial residual norm
  Real _picard_initial_norm;
  /// Full history of residual norm after evaluation of timestep_begin
  std::vector<Real> _picard_timestep_begin_norm;
  /// Full history of residual norm after evaluation of timestep_end
  std::vector<Real> _picard_timestep_end_norm;
  /// Status of Picard solve
  MoosePicardConvergenceReason _picard_status;
  ///@}

  /// Counter for number of xfem updates that have been performed in the current step
  unsigned int _xfem_update_count;
  /// Whether step should be repeated due to xfem modifying the mesh
  bool _xfem_repeat_step;

  /// Time of previous Picard solve as a subapp
  Real _previous_entering_time;
};
#endif // PICARDSOLVE_H
