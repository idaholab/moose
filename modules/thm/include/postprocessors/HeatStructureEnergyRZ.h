#pragma once

#include "HeatStructureEnergyBase.h"
#include "RZSymmetry.h"

/**
 * Computes the total energy for a cylindrical heat structure.
 */
class HeatStructureEnergyRZ : public HeatStructureEnergyBase, public RZSymmetry
{
public:
  HeatStructureEnergyRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

public:
  static InputParameters validParams();
};
