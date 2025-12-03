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
 * Project \f$ s \vec u \times \vec v \f$ onto a vector MFEM auxvariable.
 *
 * Notes:
 *  - Enforces 3D: all involved vdim must be 3.
 *  - The target variable's FE Space must be L2.
 *  - Currently supports only interior DOFs (no shared/constrained DOFs).
 */
class MFEMCrossProductAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMCrossProductAux(const InputParameters & parameters);
  ~MFEMCrossProductAux() override = default;

  void execute() override;

protected:
  /// Cross product coefficient
  mfem::VectorCrossProductCoefficient _cross;

  /// Final coefficient that applies a scaling factor to the cross product
  mfem::ScalarVectorProductCoefficient _scaled_cross;
};

#endif // MOOSE_MFEM_ENABLED
