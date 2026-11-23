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

#include "MFEMNormPostprocessor.h"

/**
 * Compute the H(curl) error for a vector variable.
 */
class MFEMHCurlError : public MFEMNormPostprocessor
{
public:
  static InputParameters validParams();

  MFEMHCurlError(const InputParameters & parameters);

  /**
   * Get the H(curl) error.
   */
  virtual PostprocessorValue getValue() const override final;

private:
  mfem::VectorCoefficient & _vec_coeff;
  mfem::VectorCoefficient & _curl_coeff;
  mfem::GridFunction & _var;
};

#endif
