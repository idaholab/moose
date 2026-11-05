//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "SolveObject.h"

class Executioner;
class RigidBodyLoadControl;

/**
 * `SolveObject` that wraps a per-time-step SNES solve (`_inner_solve`,
 * typically `FixedPointSolve`) with a Uzawa outer loop for
 * load-controlled rigid-body contact.  See `UzawaTransient` for the
 * per-iteration semantics; this class is the actual `solve()`
 * implementation.
 *
 * If `_load_control` is `nullptr` (displacement-controlled input) this
 * class is a pass-through: `solve()` returns `_inner_solve->solve()`
 * unchanged.
 */
class UzawaSolveObject : public SolveObject
{
public:
  UzawaSolveObject(Executioner & ex,
                   unsigned int outer_max_iter,
                   Real outer_abs_tol,
                   Real outer_rel_tol,
                   Real max_step,
                   unsigned int damp_max_retries,
                   bool outer_verbose);

  virtual bool solve() override;

  /// Late binding: `UzawaTransient` looks up the load-control kernel by
  /// name in `init()` (after ScalarKernels exist) and calls this to
  /// wire it in.  Must be called before `solve()`; leaving it at
  /// `nullptr` puts this object in pass-through mode.
  void setLoadControl(RigidBodyLoadControl * lc) { _load_control = lc; }

private:
  /// Handle to the ScalarKernel we toggle between ForceBalance and
  /// PinScalar.  `nullptr` = pass-through (displacement control).
  RigidBodyLoadControl * _load_control;

  const unsigned int _outer_max_iter;
  const Real _outer_abs_tol;
  const Real _outer_rel_tol;
  const Real _max_step;
  const unsigned int _damp_max_retries;
  const bool _outer_verbose;

  /// Read the current value of the load-control scalar variable
  /// (`variable` on `RigidBodyLoadControl`).
  Real readScalarValue() const;
  /// Overwrite the current value of the scalar variable and close.
  void writeScalarValue(Real new_value);
};
