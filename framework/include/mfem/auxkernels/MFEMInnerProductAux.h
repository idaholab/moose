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
 * Project \f$ s \vec u \cdot \vec v \f$ onto a scalar MFEM auxvariable.
 *
 * Notes:
 *  - The target variable's FE Space must be L2.
 *  - Currently supports only interior DOFs (no shared/constrained DOFs).
 */
class MFEMInnerProductAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMInnerProductAux(const InputParameters & parameters);
  ~MFEMInnerProductAux() override = default;

  void execute() override;

protected:
  /// Inner product coefficient
  mfem::InnerProductCoefficient _inner;

  /// Final coefficient that applies a scaling factor to the inner product
  mfem::ProductCoefficient _scaled_inner;
};

#endif // MOOSE_MFEM_ENABLED
