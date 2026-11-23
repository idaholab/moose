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
 * Vector coefficient that flattens a matrix coefficient M row by row, such that entry (i, j) of
 * M is stored at index i * M.Width() + j. This is the layout mfem::VectorDomainLFGradIntegrator
 * expects for its tensor coefficient.
 */
class MFEMFlattenedMatrixCoefficient : public mfem::VectorCoefficient
{
public:
  MFEMFlattenedMatrixCoefficient(mfem::MatrixCoefficient & mat_coef);

  /// Set the time for internally stored coefficients
  void SetTime(mfem::real_t t) override;

  /// Evaluate the flattened matrix coefficient at @a ip.
  void Eval(mfem::Vector & V,
            mfem::ElementTransformation & T,
            const mfem::IntegrationPoint & ip) override;
  using mfem::VectorCoefficient::Eval;

private:
  mfem::MatrixCoefficient * _mat_coef;
  mutable mfem::DenseMatrix _mat;
};

#endif
