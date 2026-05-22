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
 * Scalar coefficient that evaluates the magnitude of a vector coefficient.
 */
class MFEMVectorMagnitudeCoefficient : public mfem::Coefficient
{
public:
  MFEMVectorMagnitudeCoefficient(mfem::VectorCoefficient & vec_coef);

  /// Set the time for internally stored coefficients
  void SetTime(mfem::real_t t) override;

  /// Evaluate the vector coefficient magnitude at @a ip.
  mfem::real_t Eval(mfem::ElementTransformation & T, const mfem::IntegrationPoint & ip) override;

private:
  mfem::VectorCoefficient * _vec_coef;
  mutable mfem::Vector _vec;
};

#endif
