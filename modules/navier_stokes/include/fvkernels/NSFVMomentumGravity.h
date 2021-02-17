//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "FVElementalKernel.h"

/**
 * Imposes a gravitational force on the momentum equation
 */
class NSFVMomentumGravity : public FVElementalKernel
{
public:
  static InputParameters validParams();
  NSFVMomentumGravity(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  const RealVectorValue _gravity;

  /// The density
  const ADMaterialProperty<Real> & _rho;
  /// index x|y|z
  const unsigned int _index;
};
