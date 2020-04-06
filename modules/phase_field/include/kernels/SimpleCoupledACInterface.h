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

/**
 * Compute the Allen-Cahn interface term with constant Mobility and Interfacial parameter
 */
class SimpleCoupledACInterface : public Kernel
{
public:
  static InputParameters validParams();

  SimpleCoupledACInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Mobility
  const MaterialProperty<Real> & _L;
  /// Interfacial parameter
  const MaterialProperty<Real> & _kappa;
  /// Gradient of the coupled variable
  const VariableGradient & _grad_v;
  /// Index of the coupled variable
  unsigned int _v_var;
};
