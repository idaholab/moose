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
 * Imposes a gravitational force on the momentum equation in Rhie-Chow (incompressible) contexts
 */
class INSFVMomentumGravity : public FVElementalKernel
{
public:
  static InputParameters validParams();
  INSFVMomentumGravity(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// Momentum equation component (x = 0, y = 1, z = 2)
  const unsigned int _index;

  /// The gravity vector
  const RealVectorValue _gravity;

  /// The density
  const Moose::Functor<ADReal> & _rho;
};
