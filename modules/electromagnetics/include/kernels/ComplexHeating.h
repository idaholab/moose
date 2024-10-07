//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADKernel.h"

/**
 *  Supplies the heating due to the electic field in the form of 
 *  0.5 Re( conductivity * E * E^* )
 */
class ComplexHeating : public ADKernel
{
public:
  static InputParameters validParams();

  ComplexHeating(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  /// Vector variable of the real component of the E-field
  const ADVectorVariableValue & _E_real;

  /// Vector variable of the imaginary component of the E-field
  const ADVectorVariableValue & _E_imag;

  /// Real component of the material conductivity (in S/m)
  const ADMaterialProperty<Real> & _cond;

  /// Coefficient to multiply by heating term
  const Real & _scale;
};
