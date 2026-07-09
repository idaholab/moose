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
#include "MFEMQuadratureFunctionCoefficientBase.h"

#include "libmesh/ignore_warnings.h"
#include "mfem.hpp"
#include "libmesh/restore_warnings.h"

/**
 * Base class for MOOSE objects declaring an MFEM coefficient backed by precomputed values of a
 * source coefficient on quadrature points. Owns the quadrature space shared by the derived
 * scalar and vector variants and provides their common input parameters.
 */
class MFEMQuadratureFunctionBase : public MFEMObject
{
public:
  static InputParameters validParams();

  MFEMQuadratureFunctionBase(const InputParameters & parameters);

protected:
  /// Return the update policy selected by the 'updates' input parameter.
  MFEMQuadratureFunctionCoefficientBase::UpdatePolicy updatePolicy() const;

  /// Quadrature space defining the points at which the source coefficient values are stored.
  mfem::QuadratureSpace _qspace;
};

#endif
