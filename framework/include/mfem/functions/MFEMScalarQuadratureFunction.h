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

#include "MFEMObject.h"

#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

/**
 * Declares a scalar MFEM coefficient represented as a precomputed vector of values of a source
 * coefficient on quadrature points.
 */
class MFEMScalarQuadratureFunction : public MFEMObject
{
public:
  static InputParameters validParams();

  MFEMScalarQuadratureFunction(const InputParameters & parameters);

protected:
  /// Quadrature space defining the points at which the source coefficient values are stored.
  mfem::QuadratureSpace _qspace;
  /// Storage for the projected values of the source coefficient.
  mfem::QuadratureFunction _qf;
};

#endif
