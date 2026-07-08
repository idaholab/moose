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

/**
 * Shared lazy-update state for quadrature function coefficients. Holds the update policy and a
 * dirty flag marking whether the stored quadrature point values are stale and must be
 * re-projected from the source coefficient before use. Provides a common polymorphic type so
 * that scalar and vector quadrature function coefficients can be invalidated uniformly.
 */
class MFEMQuadratureFunctionCoefficientBase
{
public:
  /// Policy controlling when the stored values are re-projected from the source coefficient.
  enum class UpdatePolicy
  {
    NONE, ///< the source never changes after initialization; project exactly once
    TIME, ///< the source changes with time only; re-project when the time is set
    SOLVE ///< the source may additionally depend on solution variables; also re-project
          ///< whenever trial variables are updated (e.g. between nonlinear iterations)
  };

  MFEMQuadratureFunctionCoefficientBase(UpdatePolicy update_policy)
    : _update_policy(update_policy), _dirty(true)
  {
  }

  virtual ~MFEMQuadratureFunctionCoefficientBase() = default;

  /// Mark the stored values as stale following a change of solution variables, forcing the
  /// source to be re-projected on next use. No-op unless the update policy is SOLVE.
  void invalidate()
  {
    if (_update_policy == UpdatePolicy::SOLVE)
      _dirty = true;
  }

protected:
  /// Mark the stored values as stale following a change of time. No-op if the policy is NONE.
  void markTimeChanged()
  {
    if (_update_policy != UpdatePolicy::NONE)
      _dirty = true;
  }

  /// When the stored values are re-projected from the source coefficient.
  const UpdatePolicy _update_policy;
  /// Whether the stored values are stale and must be re-projected before use.
  bool _dirty;
};

#endif
