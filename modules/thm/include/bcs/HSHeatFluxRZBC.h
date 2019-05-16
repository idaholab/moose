#pragma once

#include "HSHeatFluxBC.h"
#include "RZSymmetry.h"

class HSHeatFluxRZBC;

template <>
InputParameters validParams<HSHeatFluxRZBC>();

/**
 * Applies a specified heat flux to the side of a cylindrical heat structure
 */
class HSHeatFluxRZBC : public HSHeatFluxBC, public RZSymmetry
{
public:
  HSHeatFluxRZBC(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual() override;
};
