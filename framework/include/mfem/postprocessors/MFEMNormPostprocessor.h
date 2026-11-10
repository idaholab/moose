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

#include "MFEMPostprocessor.h"

/**
 * Base class for postprocessors computing a norm over the mesh, such as that of the error of a
 * variable, with quadrature rules of an optional order.
 */
class MFEMNormPostprocessor : public MFEMPostprocessor
{
public:
  static InputParameters validParams();

  MFEMNormPostprocessor(const InputParameters & parameters);

protected:
  /// Quadrature rule of each element geometry, in the form MFEM's error and norm computations
  /// take, or nullptr when no quadrature order is given, which selects MFEM's default rules.
  const mfem::IntegrationRule ** getIntegrationRules() const;

private:
  /// Quadrature rules of the given order; mutable because MFEM takes them as a non-const array.
  mutable std::array<const mfem::IntegrationRule *, mfem::Geometry::NumGeom> _irs;
};

#endif
