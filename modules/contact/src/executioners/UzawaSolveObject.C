//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "UzawaSolveObject.h"

#include "Executioner.h"
#include "FEProblemBase.h"
#include "MooseVariableScalar.h"
#include "NonlinearSystemBase.h"
#include "RigidBodyLoadControl.h"
#include "SystemBase.h"

#include "libmesh/numeric_vector.h"

#include <cmath>

UzawaSolveObject::UzawaSolveObject(Executioner & ex,
                                   unsigned int outer_max_iter,
                                   Real outer_abs_tol,
                                   Real outer_rel_tol,
                                   Real max_step,
                                   unsigned int damp_max_retries,
                                   bool outer_verbose)
  : SolveObject(ex),
    _load_control(nullptr),
    _outer_max_iter(outer_max_iter),
    _outer_abs_tol(outer_abs_tol),
    _outer_rel_tol(outer_rel_tol),
    _max_step(max_step),
    _damp_max_retries(damp_max_retries),
    _outer_verbose(outer_verbose)
{
}

Real
UzawaSolveObject::readScalarValue() const
{
  // The scalar variable driven by _load_control is `variable` on that
  // kernel.  Reach it via the kernel's own `variable()` accessor.  The
  // scalar variable's `sln()` returns a VariableValue (size 1 for
  // order=FIRST).
  const auto & scalar_var = _load_control->variable();
  return scalar_var.sln()[0];
}

void
UzawaSolveObject::writeScalarValue(Real new_value)
{
  const auto & scalar_var = _load_control->variable();
  // Scalar DoFs live on the (usually last) rank that owns them.  Guard
  // the set() with a local-DoF check so non-owning ranks no-op cleanly
  // in parallel.
  auto & sys = _problem.getNonlinearSystemBase(0);
  auto & soln = sys.solution();
  const auto & dof_indices = scalar_var.dofIndices();
  const auto & dof_map = sys.dofMap();
  const auto first_local = dof_map.first_dof(_problem.mesh().comm().rank());
  const auto end_local = dof_map.end_dof(_problem.mesh().comm().rank());
  for (const auto d : dof_indices)
    if (d >= first_local && d < end_local)
      soln.set(d, new_value);
  soln.close();
  // Refresh MOOSE's cached variable values from the modified solution.
  sys.update();
}

bool
UzawaSolveObject::solve()
{
  // Pass-through when no load-control kernel is present.
  if (!_load_control)
    return _inner_solve->solve();

  Real r_s_initial = -1.0;
  Real s_current = readScalarValue();

  for (unsigned int outer = 0; outer < _outer_max_iter; ++outer)
  {
    // ------------------------------------------------------------------
    // (a) Toggle to PinScalar mode with s_pin = current s.  Primal solve
    //     will then hold s at s_current and only move (u, lambda).
    // ------------------------------------------------------------------
    _load_control->setMode(RigidBodyLoadControl::Mode::PinScalar, s_current);

    // ------------------------------------------------------------------
    // (b) Primal solve.  The inner primal SNES + KSP monitor output
    //     (`M Nonlinear |R|` / `M Linear |R|`) prints unconditionally
    //     alongside the outer Uzawa lines -- the two are useful
    //     together, and PETSc's iter-0 monitor fires before any
    //     SNESSetUpdate cancel we could hook, so a partial silence
    //     is not worth its complexity.
    // ------------------------------------------------------------------
    bool primal_ok = _inner_solve->solve();
    if (!primal_ok && _damp_max_retries > 0)
    {
      // If the primal solve failed, we can't rescue it from here (the
      // damping is on the outer scalar step, not the inner) -- report
      // failure and let the transient time-stepper cut back dt.
      if (_outer_verbose)
        _console << "Uzawa: primal solve failed on outer iter " << outer
                 << ", giving up so outer transient can cut back dt" << std::endl;
      _load_control->setMode(RigidBodyLoadControl::Mode::ForceBalance, 0.0);
      return false;
    }
    if (!primal_ok)
    {
      _load_control->setMode(RigidBodyLoadControl::Mode::ForceBalance, 0.0);
      return false;
    }

    // ------------------------------------------------------------------
    // (c) Read R_s at the converged primal state.  The most recent
    //     residual assembly (inside SNES's convergence check) populated
    //     `_cached_reaction_minus_F` in PinScalar mode -- the reaction
    //     sum is computed identically in both modes.
    // ------------------------------------------------------------------
    const Real r_s = _load_control->currentReactionMinusF();
    if (outer == 0)
      r_s_initial = std::abs(r_s);

    if (_outer_verbose)
      _console << "Uzawa outer iter " << outer << ": s = " << s_current
               << ", |R_s| = " << std::abs(r_s) << std::endl;

    // ------------------------------------------------------------------
    // (d) Convergence check on |R_s|.
    // ------------------------------------------------------------------
    if (std::abs(r_s) < _outer_abs_tol ||
        (r_s_initial > 0.0 && std::abs(r_s) < _outer_rel_tol * r_s_initial))
    {
      _load_control->setMode(RigidBodyLoadControl::Mode::ForceBalance, 0.0);
      if (_outer_verbose)
        _console << "Uzawa converged in " << (outer + 1) << " outer iters" << std::endl;
      return true;
    }

    // ------------------------------------------------------------------
    // (e) Scalar Newton step: ds = -R_s / kss_stiffness, clipped to
    //     +/- max_step (a trust-region bound so a wildly-off
    //     kss_stiffness cannot make s explode).  Use the physically-
    //     motivated Kss = kss_stiffness * total_nodal_area *
    //     sign(direction . axis_hat) already assembled in ForceBalance
    //     mode.  Since we cannot cheaply extract that from a
    //     PinScalar assembly, approximate as kss_stiffness directly --
    //     off by an O(1) area factor, but the trust region and re-
    //     iteration absorb the mismatch.
    // ------------------------------------------------------------------
    // signedKssApprox() = kss_stiffness * sign(direction . axis_hat), so
    // it carries the correct sign for `ds = -R_s / signedKss` to move s
    // in the reaction-reducing direction on either axis orientation.
    const Real signed_kss = _load_control->signedKssApprox();
    if (signed_kss == 0.0)
    {
      mooseError("UzawaSolveObject: kss_stiffness on load-control kernel '",
                 _load_control->name(),
                 "' must be > 0 for the outer scalar Newton step to make "
                 "sense.  Set kss_stiffness to (approximately) the "
                 "deformable body's Young's modulus.");
    }

    Real ds = -r_s / signed_kss;
    if (std::abs(ds) > _max_step)
      ds = (ds > 0.0 ? _max_step : -_max_step);

    s_current += ds;
    writeScalarValue(s_current);
  }

  // Fell off the end without meeting the tolerance -- outer non-
  // convergence.  Restore ForceBalance so the outer transient's own
  // post-processing sees the physical residual.
  _load_control->setMode(RigidBodyLoadControl::Mode::ForceBalance, 0.0);
  _console << "Uzawa: outer loop did not converge in " << _outer_max_iter
           << " iters, propagating failure so transient can cut back dt" << std::endl;
  return false;
}
