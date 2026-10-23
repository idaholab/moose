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
 * Projects a complex vector coefficient, optionally scaled by a real scalar coefficient, onto a
 * complex vector-valued auxvariable.
 */
class MFEMComplexVectorProjectionAux : public MFEMComplexAuxKernel
{
public:
  static InputParameters validParams();

  MFEMComplexVectorProjectionAux(const InputParameters & parameters);

  virtual ~MFEMComplexVectorProjectionAux() = default;

  virtual void execute() override;

protected:
  /// Product of the scalar coefficient and the real part of the source vector coefficient
  mfem::ScalarVectorProductCoefficient _vec_coef_real;
  /// Product of the scalar coefficient and the imaginary part of the source vector coefficient
  mfem::ScalarVectorProductCoefficient _vec_coef_imag;
};

#endif
