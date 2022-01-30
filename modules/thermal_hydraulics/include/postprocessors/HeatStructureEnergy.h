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
