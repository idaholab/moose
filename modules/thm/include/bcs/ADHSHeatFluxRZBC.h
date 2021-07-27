#pragma once

#include "ADHSHeatFluxBC.h"
#include "RZSymmetry.h"

/**
 * Applies a specified heat flux to the side of a cylindrical heat structure
 */
class ADHSHeatFluxRZBC : public ADHSHeatFluxBC, public RZSymmetry
{
public:
  ADHSHeatFluxRZBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

public:
  static InputParameters validParams();
};
