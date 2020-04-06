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
 * Compute the Cahn-Hilliard interface term with constant Mobility and Interfacial parameter
 */
class SimpleCHInterface : public Kernel
{
public:
  static InputParameters validParams();

  SimpleCHInterface(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

  ///@{ Variables for second order derivatives
  const VariableSecond & _second_u;
  const VariableTestSecond & _second_test;
  const VariablePhiSecond & _second_phi;
  ///@}

  /// Mobility
  const MaterialProperty<Real> & _M;
  /// Interfacial parameter
  const MaterialProperty<Real> & _kappa_c;
};
