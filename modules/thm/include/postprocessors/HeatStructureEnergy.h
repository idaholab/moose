#pragma once

#include "HeatStructureEnergyBase.h"

class HeatStructureEnergy;

template <>
InputParameters validParams<HeatStructureEnergy>();

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
};
