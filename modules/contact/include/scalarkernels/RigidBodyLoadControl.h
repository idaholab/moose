//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NodalScalarKernel.h"
#include "MooseEnum.h"

class Function;
class LevelSetContactor;
class NodalArea;

/**
 * Load-control constraint for a rigid-body contactor.
 *
 * The scalar variable `s` is the contactor's translation along `direction`
 * (via the contactor's `disp_x/y/z_scalar` inputs).  Newton adjusts `s` so
 * the integrated normal contact reaction equals a prescribed target:
 *
 *   R_s = Sum_i w_i * lambda_i * (n_i . direction) - F(t) = 0
 *
 * where the sum is over LM nodes on the contact sideset, w_i is the
 * tributary nodal area (from a companion NodalArea UO), lambda_i is the
 * nodal contact pressure, and n_i is the contactor's outward normal at
 * the deformed node position.
 *
 * The Jacobian assembles:
 *  * (scalar_row, lambda_col) : -w_j * (n_j . direction)   -- direct.
 *  * (lambda_row, scalar_col) : -c * (n_j . axis_hat) on gap-branch nodes,
 *    0 elsewhere -- transpose block that MOOSE's NodalKernel framework can't
 *    fill (NodalKernel has no scalar off-diagonal hook), so this kernel
 *    fills it directly.  The `c` parameter MUST match the companion
 *    RigidBodyNodalNCPKernel's `c` for the Jacobian to be consistent.
 *
 *    `axis_hat` is the POSITIVE Cartesian unit vector along the axis of
 *    the contactor's translation (i.e., the disp_[xyz]_scalar the
 *    contactor is configured with), NOT the user-supplied `direction`
 *    (which may be negative-axis).  The two agree only when
 *    `direction` is a positive-axis unit vector; when it is negative
 *    (e.g. `direction = '0 -1 0'` for a load that acts along -y), the
 *    sign of dR_lambda/ds still comes from the ACTUAL translation
 *    vector, which LevelSetContactor implements as s * axis_hat (always
 *    positive-oriented).  Using `n . direction` here -- as an older
 *    version of this class did -- produced a wrong Kls sign whenever the
 *    user supplied a negative-axis direction and made Newton fail on
 *    load-control setups where the contact geometry required it.
 */
class RigidBodyLoadControl : public NodalScalarKernel
{
public:
  static InputParameters validParams();
  RigidBodyLoadControl(const InputParameters & parameters);

  virtual void computeResidual() override;
  virtual void computeJacobian() override;

  /// Two operating modes.  See the `mode` param docstring.
  enum class Mode
  {
    ForceBalance = 0,
    PinScalar = 1
  };

  /// The reaction mismatch `Sum_i w_i * lambda_i * (n_i . direction) - F(t)`
  /// AT THE STATE from the most recent `computeResidual()` call.  This is
  /// the true (mode-independent) scalar residual the force-balance
  /// formulation would emit.  It is populated by `computeResidual` in BOTH
  /// modes (residual assembly always computes the reaction sum; only the
  /// value written to `_local_re` differs between modes).  Used by
  /// `UzawaTransient` to read `R_s` at a converged primal solve without
  /// forcing a second residual assembly.
  ///
  /// Parallel-safe: MOOSE calls `computeResidual` only on the rank that
  /// owns the scalar DoF, so `_cached_reaction_minus_F` is populated on
  /// that one rank and remains at its initialized 0 elsewhere.  The
  /// accessor does an MPI sum reduction so callers on any rank get the
  /// correct value (non-owning contributions are 0, so sum = owning
  /// rank's value).
  Real currentReactionMinusF() const;

  /// Runtime mutator used by `UzawaTransient` to flip the residual /
  /// Jacobian form between primal solves and the R_s read-back.  The
  /// input file's `mode` and `s_pin` params are treated as the initial
  /// values; after construction they are governed by whoever calls
  /// `setMode()`.
  void setMode(Mode m, Real s_pin);

  /// Read-only accessor for the current mode.
  Mode currentMode() const { return _mode; }

  /// Signed approximation of `dR_s/ds` suitable for the outer 1D Newton
  /// step in `UzawaTransient`.  Returns `kss_stiffness(t) * (direction .
  /// axis_hat)` -- positive when `direction` and `axis_hat` agree
  /// (pushing s up increases reaction, so Newton on `R_s = 0` wants
  /// `ds = -R_s/kss > 0` when reaction < target), negative otherwise.
  /// The `total_nodal_area` factor used inside the ForceBalance-mode
  /// Jacobian diagonal is deliberately dropped: the outer scalar
  /// Newton only needs a scalar effective stiffness (per unit s), not
  /// an integrated one, and the `max_step` trust region absorbs the
  /// O(1) mismatch.  Sign is what matters most; `kss_stiffness`
  /// magnitude is the user's tuning knob.
  Real signedKssApprox() const { return kssStiffness() * _direction(_axis); }

private:
  /// Deformed position of the k-th LM node (undeformed node + displacement).
  Point deformedNode(std::size_t k) const;

  /// Deformed position for a specific Node pointer, using compressed-index
  /// displacement values (index into the ordered subset of `_node_ids`
  /// nodes that were accessible and semi-local at reinit time).
  Point deformedNodePoint(const Node & node, std::size_t compressed) const;

  /// True when node `_node_ids[k]` is accessible on this rank -- either
  /// locally owned or ghosted.  ScalarKernel::computeResidual /
  /// computeJacobian run only on the rank that owns the scalar DoF and
  /// see the full boundary node list, but with an unusual partition a
  /// specific node may still be off-rank; skip those defensively rather
  /// than dereferencing a null Node pointer.
  bool nodeIsAccessible(std::size_t k) const;

  const Function & _force;
  const LevelSetContactor & _contactor;
  const NodalArea & _nodal_area;
  Point _direction;
  /// Return the effective contact stiffness at the current simulation time.
  /// See `_kss_stiffness_constant` / `_kss_stiffness_function` for the
  /// physical meaning and preconditioning rationale.  Evaluated at (t,
  /// origin) if a function was supplied; otherwise the constant value.
  Real kssStiffness() const;

  /// Constant variant of the effective contact stiffness (Young's-modulus
  /// scale) used to fabricate a nonzero (scalar_row, scalar_col) Jacobian
  /// entry.  The exact `dR_s/ds` is zero in this formulation (F(t) does not
  /// depend on s, and the reaction depends on s only through Kls * Kpp^-1 *
  /// Ksl, which we don't have in closed form here), so with `Kss=0` the
  /// linear solve relies on PETSc pivot shifts and Newton overshoots
  /// dramatically on flat contact patches where the true Schur-complement
  /// Kss is O(K_material * A_contact).  This is a JACOBIAN-ONLY
  /// modification: the residual is unchanged, so the physical fixed point
  /// (R_s = 0) is unchanged -- only the intermediate Newton iterates
  /// differ.  Set to 0 (default) to preserve the original formulation
  /// exactly.  Mutually exclusive with `_kss_stiffness_function`.
  const Real _kss_stiffness_constant;
  /// Function-of-time variant of the same stiffness.  When set, its value
  /// at (current time, origin) replaces the constant on every Jacobian
  /// assembly.  Useful when the appropriate preconditioning shift changes
  /// over the load history -- e.g. much larger during initial impact than
  /// during a well-established plastic patch.  Nullptr when the user
  /// supplied a constant `kss_stiffness` instead.
  const Function * const _kss_stiffness_function;
  /// Cartesian axis index (0=x, 1=y, 2=z) that the contactor's translation
  /// scalar drives.  Determined from `_direction` in the ctor (which
  /// enforces `_direction` to be aligned with a Cartesian axis).  The
  /// contactor translates by `s * axis_hat` where `axis_hat` is the
  /// positive unit vector for this axis, independent of the sign of
  /// `_direction` -- that positive-axis translation is what enters
  /// dR_lambda/ds in computeJacobian().
  unsigned int _axis;
  const Real _c;

  const unsigned int _lm_var_num;
  const VariableValue & _lambda; ///< Per-node LM values after reinitNodes.

  const unsigned int _ndisp;
  std::vector<unsigned int> _disp_var_num;
  std::vector<const VariableValue *> _disp;

  /// Operating mode.  Initialized from the input's `mode` param;
  /// thereafter set by `setMode()` (used by `UzawaTransient`).
  Mode _mode;
  /// Target value for the scalar `s` when `_mode = PinScalar`.
  /// Initialized from the input's `s_pin` param; thereafter set by
  /// `setMode()`.
  Real _s_pin;

  /// Cache of the force-balance residual at the most recent
  /// computeResidual() call.  See `currentReactionMinusF()`.
  Real _cached_reaction_minus_F;
};
