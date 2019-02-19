#ifndef HEATSTRUCTUREENERGYRZ_H
#define HEATSTRUCTUREENERGYRZ_H

#include "HeatStructureEnergyBase.h"
#include "RZSymmetry.h"

class HeatStructureEnergyRZ;

template <>
InputParameters validParams<HeatStructureEnergyRZ>();

/**
 * Computes the total energy for a cylindrical heat structure.
 */
class HeatStructureEnergyRZ : public HeatStructureEnergyBase, public RZSymmetry
{
public:
  HeatStructureEnergyRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();
};

#endif
