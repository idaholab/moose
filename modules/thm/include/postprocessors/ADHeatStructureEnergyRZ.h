#pragma once

#include "ADHeatStructureEnergyBase.h"
#include "RZSymmetry.h"

/**
 * Computes the total energy for a cylindrical heat structure.
 */
class ADHeatStructureEnergyRZ : public ADHeatStructureEnergyBase, public RZSymmetry
{
public:
  ADHeatStructureEnergyRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

public:
  static InputParameters validParams();
};
