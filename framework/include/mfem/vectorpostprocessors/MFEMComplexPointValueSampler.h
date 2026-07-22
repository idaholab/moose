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

#include "MFEMComplexValueSamplerBase.h"

/**
 * Samples a complex-valued MFEM variable at specific points.
 * Outputs real and imaginary parts as separate VPP columns.
 */
class MFEMComplexPointValueSampler : public MFEMComplexValueSamplerBase
{
public:
  static InputParameters validParams();

  MFEMComplexPointValueSampler(const InputParameters & parameters);
};

#endif // MOOSE_MFEM_ENABLED
