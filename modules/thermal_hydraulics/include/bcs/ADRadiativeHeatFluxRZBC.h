#pragma once

#include "ADRadiativeHeatFluxBC.h"
#include "RZSymmetry.h"

/**
 * Radiative heat transfer boundary condition for a cylindrical heat structure
 */
class ADRadiativeHeatFluxRZBC : public ADRadiativeHeatFluxBC, public RZSymmetry
{
public:
  ADRadiativeHeatFluxRZBC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

public:
  static InputParameters validParams();
};
