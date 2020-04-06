//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ElementAverageValue.h"

/**
 * Calculates the total volume fraction of the coupled solid mineral
 * species (volume of mineral species / volume of model)
 */
class TotalMineralVolumeFraction : public ElementAverageValue
{
public:
  static InputParameters validParams();

  TotalMineralVolumeFraction(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Molar volume of coupled mineral species (molar mass / density)
  const Real _molar_volume;
};
