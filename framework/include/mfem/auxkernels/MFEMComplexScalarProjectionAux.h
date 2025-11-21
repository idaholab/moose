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

#include "MFEMComplexAuxKernel.h"

/**
 * Projects a scalar coefficient onto a scalar-valued auxvariable.
 */
class MFEMComplexScalarProjectionAux : public MFEMComplexAuxKernel
{
public:
  static InputParameters validParams();

  MFEMComplexScalarProjectionAux(const InputParameters & parameters);

  virtual ~MFEMComplexScalarProjectionAux() = default;

  virtual void execute() override;

protected:
  /// Reference to source coefficient.
  mfem::Coefficient & _coef_real;
  mfem::Coefficient & _coef_imag;
};

#endif
