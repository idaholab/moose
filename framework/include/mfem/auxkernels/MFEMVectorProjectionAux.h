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

#include "MFEMAuxKernel.h"

/**
 * Projects a vector coefficient, optionally scaled by a scalar coefficient, onto a vector-valued
 * auxvariable.
 */
class MFEMVectorProjectionAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMVectorProjectionAux(const InputParameters & parameters);

  virtual ~MFEMVectorProjectionAux() = default;

  virtual void execute() override;

protected:
  /// Product of the scalar and vector source coefficients.
  mfem::ScalarVectorProductCoefficient _vec_coef;
};

#endif
