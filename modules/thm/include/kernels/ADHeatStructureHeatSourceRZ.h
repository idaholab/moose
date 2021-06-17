#pragma once

#include "ADHeatStructureHeatSource.h"
#include "RZSymmetry.h"

/**
 * Forcing function used in the heat conduction equation in arbitrary RZ symmetry
 */
class ADHeatStructureHeatSourceRZ : public ADHeatStructureHeatSource, public RZSymmetry
{
public:
  ADHeatStructureHeatSourceRZ(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();
  virtual Real computeQpJacobian();

public:
  static InputParameters validParams();
};
