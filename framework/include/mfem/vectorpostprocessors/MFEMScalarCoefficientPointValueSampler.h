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

#include "MFEMSamplerBase.h"

/**
 * Samples a real scalar MFEM coefficient at specified points.
 */
class MFEMScalarCoefficientPointValueSampler : public MFEMSamplerBase
{
public:
  static InputParameters validParams();

  MFEMScalarCoefficientPointValueSampler(const InputParameters & parameters);

  /// Checks if not a quadrature-function-backed coefficient.
  void initialSetup() override;

  /// Evaluates the coefficient in each point's owning element.
  void execute() override;

  /// Copies interpolated values into the VPP vectors.
  void finalize() override;

private:
  /// Scalar coefficient being sampled, resolved after all coefficient-declaring objects exist.
  mfem::Coefficient * _coefficient;
  /// Values evaluated on owning ranks and returned to the querying ranks.
  mfem::Vector _interp_vals;
  /// VectorPostprocessor output column for the coefficient.
  VectorPostprocessorValue & _declared_vals;
};

#endif // MOOSE_MFEM_ENABLED
