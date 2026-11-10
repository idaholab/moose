//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifdef MOOSE_MFEM_ENABLED

#pragma once

#include "MFEMNonlinearSolverBase.h"

namespace Moose::MFEM
{
/**
 * mfem::NewtonSolver applying a constant damping factor to the Newton update, optionally
 * shortening it further with a backtracking line search on the residual norm.
 */
class DampedNewtonSolver : public mfem::NewtonSolver
{
public:
  DampedNewtonSolver(MPI_Comm comm) : mfem::NewtonSolver(comm) {}

  void SetOperator(const mfem::Operator & op) override;

  /// Set the fraction of the full Newton update applied. The backtracking line search, when
  /// enabled, starts from this trial step.
  void SetDampingFactor(mfem::real_t damping_factor) { _damping_factor = damping_factor; }

  /// Enable a backtracking line search taking at most @a max_its trial steps, each a factor
  /// @a contraction_factor shorter than the last, accepting the first whose residual norm
  /// satisfies the sufficient decrease condition with coefficient @a sufficient_decrease.
  void SetBacktracking(unsigned int max_its,
                       mfem::real_t contraction_factor,
                       mfem::real_t sufficient_decrease);

  /**
   * Return the fraction of the Newton update to apply at the iterate @a x. Without
   * backtracking this is the constant damping factor. With backtracking it is the first trial
   * step t, starting from the damping factor, satisfying
   * ||F(x - t c) - b|| <= (1 - sufficient_decrease * t) * ||F(x) - b||,
   * or 0 if no trial step satisfies it, which interrupts the Newton iteration.
   */
  mfem::real_t ComputeScalingFactor(const mfem::Vector & x, const mfem::Vector & b) const override;

protected:
  /// Fraction of the full Newton update applied, and the line search's initial trial step.
  mfem::real_t _damping_factor = 1.0;
  /// Maximum number of trial steps taken by the line search; 0 disables backtracking.
  unsigned int _ls_max_its = 0;
  /// Factor a rejected trial step is multiplied by to obtain the next one.
  mfem::real_t _ls_contraction_factor = 0.5;
  /// Coefficient of the sufficient decrease (Armijo) condition on the residual norm.
  mfem::real_t _ls_sufficient_decrease = 1.0e-4;
  /// Trial iterate x - t c evaluated by the line search.
  mutable mfem::Vector _ls_x_trial;
  /// Residual at the trial iterate.
  mutable mfem::Vector _ls_r_trial;
};
} // namespace Moose::MFEM

/**
 * MooseObject wrapper for mfem::NewtonSolver-backed nonlinear solves.
 */
class MFEMNewtonNonlinearSolver : public Moose::MFEM::NonlinearSolverBase
{
public:
  static InputParameters validParams();

  MFEMNewtonNonlinearSolver(const InputParameters & parameters);

  void ConstructSolver() override;

  void SetLinearSolver(mfem::Solver & solver) override;
  bool RequiresGradient() const override { return true; }
  bool RequiresExternalLinearSolver() const override { return true; }
};

#endif
