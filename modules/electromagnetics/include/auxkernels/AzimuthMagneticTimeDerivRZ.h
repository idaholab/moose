//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "AuxKernel.h"

/**
 *  Computes the time derivative of the azimuthal component
 *  of the magnetic field assuming cylindrical electric field. The electric field can
 *  be supplied as a vector or scalar components.
 */
class AzimuthMagneticTimeDerivRZ : public AuxKernel
{
public:
  static InputParameters validParams();

  AzimuthMagneticTimeDerivRZ(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

private:
  /// True if the vector of the electric field was provided
  const bool _is_efield_vector;

  /// True if both the x- & y- component of the electric field were provided
  const bool _is_efield_scalar;

  /// Curl of the electric field vector
  const VectorVariableCurl & _efield_curl;

  /// Gradient of the x-component of the electric field
  const VariableGradient & _efield_x_grad;

  /// Gradient of the y-component of the electric field
  const VariableGradient & _efield_y_grad;
};
