#pragma once

#include "HeatStructureEnergyBase.h"

/**
 * Computes the total energy for a 3D heat structure
 */
class HeatStructureEnergy3D : public HeatStructureEnergyBase
{
public:
  HeatStructureEnergy3D(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

public:
  static InputParameters validParams();
};
