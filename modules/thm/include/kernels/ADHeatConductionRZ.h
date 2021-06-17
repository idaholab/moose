#pragma once

#include "HeatConduction.h"
#include "RZSymmetry.h"

/**
 * Heat conduction kernel in arbitrary RZ symmetry
 */
class ADHeatConductionRZ : public HeatConductionKernel, public RZSymmetry
{
public:
  ADHeatConductionRZ(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual();

public:
  static InputParameters validParams();
};
