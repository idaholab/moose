//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADIntegratedBC.h"

/**
 *  Represents the boundary condition for a first order Robin-style Absorbing/Port
 *  boundary for scalar variables. NOTE, assumes plane waves!
 */
class EMRobinBC : public ADIntegratedBC
{
public:
  static InputParameters validParams();

  EMRobinBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

private:
  /// Real component of the electric field
  const ADVariableValue & _field_real;

  /// Imaginary component of the electric field
  const ADVariableValue & _field_imag;

  /// Enum for selection of real or imaginary component of the field wave
  const MooseEnum _component;

  /// Real component of the function coefficient representing the wavenumber
  const Function & _func_real;

  /// Imaginary component of the function coefficient representing the wavenumber
  const Function & _func_imag;

  /// Real component of the incoming wave function amplitude
  const Function & _profile_func_real;

  /// Imaginary component of the incoming wave function amplitude
  const Function & _profile_func_imag;

  /// Real component of the constant coefficient representing the wavenumber
  const Real _coeff_real;

  /// Imaginary component of the constant coefficient representing the wavenumber
  const Real _coeff_imag;

  /// Scalar value representing the sign of the term in the weak form
  const MooseEnum _sign;

  /// Enum for selection of boundary condition mode: absorbing or port (Default = port)
  const MooseEnum _mode;
};
