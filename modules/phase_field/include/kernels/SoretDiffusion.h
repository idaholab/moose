//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"

// Forward Declaration

/**
 * SoretDiffusion adds the soret effect in the split form of the Cahn-Hilliard
 * equation.
 */
class SoretDiffusion : public Kernel
{
public:
  static InputParameters validParams();

  SoretDiffusion(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);
  virtual Real computeQpCJacobian();

  /// int label for temperature variable
  unsigned int _T_var;

  /// Coupled variable for the temperature
  const VariableValue & _T;

  /// Variable gradient for temperature
  const VariableGradient & _grad_T;

  /// is the kernel used in a coupled form?
  const bool _is_coupled;

  /// int label for the Concentration
  unsigned int _c_var;

  /// Variable value for the concentration
  const VariableValue & _c;

  /// Diffusivity material property
  const MaterialProperty<Real> & _D;

  /// Heat of transport material property
  const MaterialProperty<Real> & _Q;

  /// Boltzmann constant
  const Real _kB;
};
