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
 * Declares a vector MFEM coefficient represented as a precomputed vector of values of a source
 * vector coefficient on quadrature points.
 */
class MFEMVectorQuadratureFunction : public MFEMQuadratureFunctionBase
{
public:
  static InputParameters validParams();

  MFEMVectorQuadratureFunction(const InputParameters & parameters);

protected:
  /// Storage for the projected values of the source coefficient.
  mfem::QuadratureFunction _qf;
};

#endif
