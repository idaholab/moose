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

// The class derives from mfem::VectorQuadratureFunctionCoefficient, so the complete type is
// required.
#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

/**
 * Vector coefficient holding precomputed values of a source vector coefficient at the quadrature
 * points of a QuadratureFunction. The stored values are (re)projected lazily: evaluation
 * triggers a projection of the source coefficient only if the values have been invalidated
 * since the last projection.
 */
class MFEMVectorQuadratureFunctionCoefficient : public mfem::VectorQuadratureFunctionCoefficient,
                                                public MFEMQuadratureFunctionCoefficientBase
{
public:
  MFEMVectorQuadratureFunctionCoefficient(mfem::VectorCoefficient & source,
                                          mfem::QuadratureFunction & qf,
                                          UpdatePolicy update_policy,
                                          const std::string & name);

  /// Set the time for the coefficient, invalidating the stored values unless the update
  /// policy is NONE.
  void SetTime(mfem::real_t t) override;

  using mfem::VectorQuadratureFunctionCoefficient::Eval;
  /// Return the stored values at @a ip, re-projecting the source first if invalidated.
  void Eval(mfem::Vector & V,
            mfem::ElementTransformation & T,
            const mfem::IntegrationPoint & ip) override;

  /// Copy the stored values into @a qf, re-projecting the source first if invalidated.
  void Project(mfem::QuadratureFunction & qf) override;

private:
  /// Project the source coefficient into the quadrature function if invalidated.
  void Refresh();

  /// Source coefficient the stored values are projected from.
  mfem::VectorCoefficient & _source;
  /// Storage for the projected values, shared with the owning MOOSE object.
  mfem::QuadratureFunction & _qf;
};

#endif
