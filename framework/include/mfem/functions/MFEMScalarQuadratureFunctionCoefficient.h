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

#include "MFEMQuadratureFunctionCoefficientBase.h"

// The class derives from mfem::QuadratureFunctionCoefficient, so the complete type is required.
#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

/**
 * Scalar coefficient holding precomputed values of a source coefficient at the quadrature
 * points of a QuadratureFunction. The stored values are (re)projected lazily: evaluation
 * triggers a projection of the source coefficient only if the values have been invalidated
 * since the last projection.
 */
class MFEMScalarQuadratureFunctionCoefficient : public mfem::QuadratureFunctionCoefficient,
                                                public MFEMQuadratureFunctionCoefficientBase
{
public:
  MFEMScalarQuadratureFunctionCoefficient(mfem::Coefficient & source,
                                          mfem::QuadratureFunction & qf,
                                          UpdatePolicy update_policy,
                                          const std::string & name);

  /// Set the time for the coefficient, invalidating the stored values unless the update
  /// policy is NONE.
  void SetTime(mfem::real_t t) override;

  /// Return the stored value at @a ip, re-projecting the source first if invalidated.
  mfem::real_t Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip) override;

  /// Copy the stored values into @a qf, re-projecting the source first if invalidated.
  void Project(mfem::QuadratureFunction & qf) override;

private:
  /// Project the source coefficient into the quadrature function if invalidated.
  void Refresh();

  /// Source coefficient the stored values are projected from.
  mfem::Coefficient & _source;
  /// Storage for the projected values, shared with the owning MOOSE object.
  mfem::QuadratureFunction & _qf;
};

#endif
