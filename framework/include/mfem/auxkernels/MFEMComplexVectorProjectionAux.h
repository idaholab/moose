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
 * Projects a vector coefficient onto a vector-valued auxvariable.
 */
class MFEMComplexVectorProjectionAux : public MFEMComplexAuxKernel
{
public:
  static InputParameters validParams();

  MFEMComplexVectorProjectionAux(const InputParameters & parameters);

  virtual ~MFEMComplexVectorProjectionAux() = default;

  virtual void execute() override;

protected:
  /// Reference to source coefficient.
  mfem::VectorCoefficient & _vec_coef_real;
  mfem::VectorCoefficient & _vec_coef_imag;
};

#endif
