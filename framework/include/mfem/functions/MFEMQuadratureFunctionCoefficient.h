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

#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

/**
 * Scalar coefficient holding precomputed values of a source coefficient at the quadrature
 * points of a QuadratureFunction. The stored values are (re)projected lazily: evaluation
 * triggers a projection of the source coefficient only if the values have been invalidated
 * since the last projection.
 */
class MFEMQuadratureFunctionCoefficient : public mfem::QuadratureFunctionCoefficient
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

  MFEMQuadratureFunctionCoefficient(mfem::Coefficient & source,
                                    mfem::QuadratureFunction & qf,
                                    UpdatePolicy update_policy);

  /// Mark the stored values as stale following a change of solution variables, forcing the
  /// source to be re-projected on next use. No-op unless the update policy is SOLVE.
  void invalidate()
  {
    if (_update_policy == UpdatePolicy::SOLVE)
      _dirty = true;
  }

  /// Set the time for the coefficient, invalidating the stored values unless the update
  /// policy is NONE.
  void SetTime(mfem::real_t t) override;

  /// Return the stored value at @a ip, re-projecting the source first if invalidated.
  mfem::real_t Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip) override;

  /// Copy the stored values into @a qf, re-projecting the source first if invalidated.
  void Project(mfem::QuadratureFunction & qf) override;

private:
  /// Project the source coefficient into the quadrature function if invalidated.
  void refresh();

  /// Source coefficient the stored values are projected from.
  mfem::Coefficient & _source;
  /// Storage for the projected values, shared with the owning MOOSE object.
  mfem::QuadratureFunction & _qf;
  /// When the stored values are re-projected from the source coefficient.
  const UpdatePolicy _update_policy;
  /// Whether the stored values are stale and must be re-projected before use.
  bool _dirty;
};

#endif
