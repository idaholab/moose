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

#include "MFEMVectorPostprocessor.h"
#include "MFEMGeneralUserObject.h"

/**
 * Compute the L2 error for a variable.
 */
class MFEMEigenvaluesPostprocessor : public MFEMVectorPostprocessor
{
public:
  static InputParameters validParams();

  MFEMEigenvaluesPostprocessor(const InputParameters & parameters);

  virtual void initialize() override;
  virtual void execute() override;

  std::reference_wrapper<VectorPostprocessorValue> _eigenvalues;
};

#endif
