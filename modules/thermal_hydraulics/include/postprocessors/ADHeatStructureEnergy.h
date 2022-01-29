#pragma once

#include "ADHeatStructureEnergyBase.h"

/**
 * Computes the total energy for a plate heat structure.
 */
class ADHeatStructureEnergy : public ADHeatStructureEnergyBase
{
public:
  ADHeatStructureEnergy(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Depth of the heat structure
  const Real _plate_depth;

public:
  static InputParameters validParams();
};
