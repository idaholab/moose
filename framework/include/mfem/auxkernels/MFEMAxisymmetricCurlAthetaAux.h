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
 * Class to compute the axisymmetric curl associated with
 * an azimuthal scalar field A_theta.
 *
 * For A_theta(r,z), this computes
 *
 *   B_r = -dA_theta/dz
 *   B_z =  dA_theta/dr + A_theta/r
 *
 * using the regularized inverse-r coefficient declared by an
 * MFEMCoordinateTransformations function object with coord_type = RZ.
 */
class MFEMAxisymmetricCurlAthetaAux : public MFEMAuxKernel
{
public:
  static InputParameters validParams();

  MFEMAxisymmetricCurlAthetaAux(const InputParameters & parameters);

  virtual ~MFEMAxisymmetricCurlAthetaAux() = default;

  /// Computes the auxvariable.
  virtual void execute() override;

protected:
  /// Name of source MFEMVariable storing A_theta.
  const VariableName _source_var_name;

  /// Reference to source scalar gridfunction.
  const mfem::ParGridFunction & _source_var;

  /// Name of the MFEMCoordinateTransformations function object.
  const FunctionName _coordinate_function;

  /// Derived inverse-r coefficient name "<coordinate_function>_inv_r".
  const std::string _inv_r_coefficient;
};

#endif
