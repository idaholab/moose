//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HeatStructureEnergyBase.h"

/**
 * Computes the total energy for a plate heat structure.
 */
class HeatStructureEnergy : public HeatStructureEnergyBase
{
public:
  HeatStructureEnergy(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Depth of the heat structure
  const Real _plate_depth;

public:
  static InputParameters validParams();
};
