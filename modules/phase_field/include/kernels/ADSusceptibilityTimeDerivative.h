//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ADTimeDerivative.h"

/**
 * This calculates the time derivative for a variable multiplied by a generalized susceptibility
 */
class ADSusceptibilityTimeDerivative : public ADTimeDerivative
{
public:
  static InputParameters validParams();

  ADSusceptibilityTimeDerivative(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual() override;

  /// susceptibility
  const ADMaterialProperty<Real> & _Chi;
};
