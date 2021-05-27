#pragma once

#include "HeatStructureHeatSource.h"
#include "RZSymmetry.h"

/**
 * Forcing function used in the heat conduction equation in arbitrary RZ symmetry
 */
class HeatStructureHeatSourceRZ : public HeatStructureHeatSource, public RZSymmetry
{
public:
  HeatStructureHeatSourceRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();

public:
  static InputParameters validParams();
};
