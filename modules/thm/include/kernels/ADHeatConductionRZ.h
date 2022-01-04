#pragma once

#include "ADHeatConduction.h"
#include "RZSymmetry.h"

/**
 * Heat conduction kernel in arbitrary RZ symmetry
 */
class ADHeatConductionRZ : public ADHeatConduction, public RZSymmetry
{
public:
  ADHeatConductionRZ(const InputParameters & parameters);

protected:
  virtual ADRealVectorValue precomputeQpResidual();

public:
  static InputParameters validParams();
};
