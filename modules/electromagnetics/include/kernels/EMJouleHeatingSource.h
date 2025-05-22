//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 *  NOTE: This kernel will be deprecated in the near future (10/01/2025)
 *        in favor of exclusively using the Heat Transfer module's
 *        'ADJouleHeatingSource' for coupling electromagnetics to heat transfer problems.
 */

/**
 *  Supplies the heating due to the electric field (E) in the form of
 *  0.5 Re( conductivity * E * E^* )
 *  where E^* is the complex conjugate of the electric field.
 */
class EMJouleHeatingSource : public ADKernel
{
public:
  static InputParameters validParams();

  EMJouleHeatingSource(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Vector variable of the real component of the electric field
  const ADVectorVariableValue & _E_real;

  /// Vector variable of the imaginary component of the electric field
  const ADVectorVariableValue & _E_imag;

  /// Real component of the material conductivity (in S/m)
  const ADMaterialProperty<Real> & _cond;

  /// Coefficient to multiply by heating term
  const Real _scale;
};
