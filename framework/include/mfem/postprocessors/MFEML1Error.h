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
 * Compute the L1 error for a scalar variable.
 */
class MFEML1Error : public MFEMNormPostprocessor
{
public:
  static InputParameters validParams();

  MFEML1Error(const InputParameters & parameters);

  /**
   * Get the L1 error.
   */
  virtual PostprocessorValue getValue() const override final;

private:
  mfem::Coefficient & _coeff;
  mfem::GridFunction & _var;
};

#endif
