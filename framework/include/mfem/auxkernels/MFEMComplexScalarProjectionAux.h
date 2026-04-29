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

namespace Moose::MFEM
{
/**
 * Projects a scalar coefficient onto a scalar-valued auxvariable.
 */
class ComplexScalarProjectionAux : public ComplexAuxKernel
{
public:
  static InputParameters validParams();

  ComplexScalarProjectionAux(const InputParameters & parameters);

  virtual ~ComplexScalarProjectionAux() = default;

  virtual void execute() override;

protected:
  /// Reference to source coefficient for the real part
  mfem::Coefficient & _coef_real;
  /// Reference to source coefficient for the imaginary part
  mfem::Coefficient & _coef_imag;
};

} // namespace Moose::MFEM
#endif
