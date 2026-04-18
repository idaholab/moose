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

namespace Moose::MFEM
{
/**
 * Project \f$ s \vec u \cdot \vec v \f$ onto a scalar MFEM auxvariable.
 *
 * Notes:
 *  - The target variable's FE Space must be L2.
 *  - Currently supports only interior DOFs (no shared/constrained DOFs).
 */
class InnerProductAux : public AuxKernel
{
public:
  static InputParameters validParams();

  InnerProductAux(const InputParameters & parameters);
  ~InnerProductAux() override = default;

  void execute() override;

protected:
  /// Inner product coefficient
  mfem::InnerProductCoefficient _inner;

  /// Final coefficient that applies a scaling factor to the inner product
  mfem::ProductCoefficient _scaled_inner;
};

} // namespace Moose::MFEM
#endif // MOOSE_MFEM_ENABLED
