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
 * Projects a scalar coefficient onto a scalar-valued aux variable.
 */
class MFEMScalarProjectionAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMScalarProjectionAux(const InputParameters & parameters);

  virtual ~MFEMScalarProjectionAux() = default;

  virtual void execute() override;

protected:
  /// Reference to source coefficient.
  mfem::Coefficient & _coef;
};

#endif
