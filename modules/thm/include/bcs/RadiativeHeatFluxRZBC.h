#pragma once

#include "RadiativeHeatFluxBC.h"
#include "RZSymmetry.h"

class RadiativeHeatFluxRZBC;

template <>
InputParameters validParams<RadiativeHeatFluxRZBC>();

/**
 * Radiative heat transfer boundary condition for a cylindrical heat structure
 */
class RadiativeHeatFluxRZBC : public RadiativeHeatFluxBC, public RZSymmetry
{
public:
  RadiativeHeatFluxRZBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
  virtual Real computeQpJacobian() override;
};
