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

#include "MFEMQuadratureFunctionBase.h"

/**
 * Declares a scalar MFEM coefficient represented as a precomputed vector of values of a source
 * coefficient on quadrature points.
 */
class MFEMScalarQuadratureFunction : public MFEMQuadratureFunctionBase
{
public:
  static InputParameters validParams();

  MFEMScalarQuadratureFunction(const InputParameters & parameters);

protected:
  /// Storage for the projected values of the source coefficient.
  mfem::QuadratureFunction _qf;
};

#endif
