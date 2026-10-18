//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Transient.h"

#include <memory>

class SolveObject;
class RigidBodyLoadControl;

/**
 * Transient executioner subclass that runs a Uzawa outer loop around the
 * standard per-time-step SNES solve.  At each time step:
 *   1. The normal `Predictor` (if any) fires once, as in plain `Transient`.
 *   2. The outer Uzawa loop runs `outer_max_iter` times:
 *        a. Flip the referenced `RigidBodyLoadControl` scalar kernel to
 *           `PinScalar` mode with `s_pin = <current scalar value>`.
 *        b. Run the standard SNES via `_fixed_point_solve->solve()` --
 *           driven by whatever petsc_options the input file supplies
 *           (SNESVINEWTONSSLS + semismooth + bounds is recommended).
 *        c. Flip the kernel back to `ForceBalance` mode and read the
 *           cached `R_s = Sum w * lambda * (n.dir) - F(t)` at the
 *           converged primal state (no extra assembly -- the value is
 *           populated as a side-effect of step (b)'s convergence check).
 *        d. If `|R_s| < outer_abs_tol` (or `< outer_rel_tol * |R_s|_0`),
 *           break -- outer converged.
 *        e. Otherwise take a 1D scalar Newton step
 *              `ds = -R_s / kss_stiffness`
 *           clipped to `|ds| <= max_step`; write the new `s` back to the
 *           scalar variable and repeat.
 *   3. If outer_max_iter is exhausted without convergence, propagate
 *      `lastSolveConverged() = false` so the transient time-stepper
 *      cuts back.
 *
 * Displacement-controlled inputs (no `RigidBodyLoadControl` scalar kernel)
 * are handled by dropping through to plain `Transient` behavior: the outer
 * loop runs once, no toggling, no scalar update.
 *
 * See `uzawa_solver_plan.md` for the full design and phasing rationale.
 */
class UzawaTransient : public Transient
{
public:
  static InputParameters validParams();
  UzawaTransient(const InputParameters & parameters);
  virtual ~UzawaTransient();

  virtual void init() override;

  /// Override so the outer time-stepper drives the Uzawa outer loop
  /// (via `_uzawa_solve`), which in turn calls the original
  /// `_fixed_point_solve` for each primal solve.
  virtual SolveObject * timeStepSolveObject() override;

private:
  /// Name of the [ScalarKernels/*] entry with type = RigidBodyLoadControl
  /// to toggle between ForceBalance and PinScalar between outer iters.
  /// Empty string => plain Transient behavior (no outer loop, no toggle).
  const std::string _load_control_kernel_name;

  /// Outer-loop tuning knobs, all read from the [Uzawa] sub-block.
  const unsigned int _outer_max_iter;
  const Real _outer_abs_tol;
  const Real _outer_rel_tol;
  const Real _max_step;
  const unsigned int _damp_max_retries;
  const bool _outer_verbose;

  /// The wrapper SolveObject that runs the outer loop.  Constructed in
  /// `init()` (after MOOSE has instantiated ScalarKernels so the
  /// `RigidBodyLoadControl` handle can be resolved).  Owns nothing --
  /// its `_inner_solve` is set to point at the base `_fixed_point_solve`.
  std::unique_ptr<SolveObject> _uzawa_solve;
};
