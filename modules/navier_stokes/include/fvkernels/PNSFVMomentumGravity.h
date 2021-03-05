//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "NSFVMomentumGravity.h"

/**
 * Imposes a gravitational force on the momentum equation in porous media.
 */
class PNSFVMomentumGravity : public NSFVMomentumGravity
{
public:
  static InputParameters validParams();
  PNSFVMomentumGravity(const InputParameters & params);

protected:
  ADReal computeQpResidual() override;

  /// the porosity
  const MaterialProperty<Real> & _eps;
};
