#pragma once

#include "ADHeatConductionTimeDerivative.h"
#include "RZSymmetry.h"

/**
 * Time derivative kernel used by heat conduction equation in arbitrary RZ symmetry
 */
class ADHeatConductionTimeDerivativeRZ : public ADHeatConductionTimeDerivative, public RZSymmetry
{
public:
  ADHeatConductionTimeDerivativeRZ(const InputParameters & parameters);

protected:
  virtual ADReal precomputeQpResidual();

public:
  static InputParameters validParams();
};
