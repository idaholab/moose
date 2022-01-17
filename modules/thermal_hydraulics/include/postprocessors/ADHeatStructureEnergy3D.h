#pragma once

#include "ADHeatStructureEnergyBase.h"

/**
 * Computes the total energy for a 3D heat structure
 */
class ADHeatStructureEnergy3D : public ADHeatStructureEnergyBase
{
public:
  ADHeatStructureEnergy3D(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

public:
  static InputParameters validParams();
};
