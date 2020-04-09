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
#include "DerivativeMaterialInterface.h"

/**
 * Solves Cahn-Hilliard equation using
 * chemical potential as non-linear variable
 **/
class CHSplitConcentration : public DerivativeMaterialInterface<Kernel>
{
public:
  static InputParameters validParams();

  CHSplitConcentration(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int);

  /// Mobility property name
  MaterialPropertyName _mobility_name;

  const MaterialProperty<RealTensorValue> & _mobility;
  const MaterialProperty<RealTensorValue> & _dmobility_dc;

  /// Chemical potential variable
  const unsigned int _mu_var;
  const VariableGradient & _grad_mu;
};
