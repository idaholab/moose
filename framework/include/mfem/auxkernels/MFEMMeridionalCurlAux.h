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
 * Class to compute the meridional curl associated with an azimuthal scalar field A_theta.
 *
 * For a 2D meridional mesh with coordinates
 *
 *   x = r
 *   y = z
 *
 * this computes
 *
 *   B_r = -dA_theta/dz
 *   B_z =  dA_theta/dr + A_theta/r
 *
 * using r = x directly.
 */
class MFEMMeridionalCurlAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMMeridionalCurlAux(const InputParameters & parameters);

  virtual ~MFEMMeridionalCurlAux() = default;

  /// Computes the auxvariable.
  virtual void execute() override;

protected:
  /// Name of source MFEMVariable storing A_theta.
  const VariableName _source_var_name;

  /// Reference to source scalar grid function.
  const mfem::ParGridFunction & _source_var;
};

#endif
