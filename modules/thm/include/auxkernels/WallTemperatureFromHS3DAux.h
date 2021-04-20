#pragma once

#include "AuxKernel.h"
#include "LayeredSideAverage.h"

/**
 * Gets the average wall temperature over each layer of a 3D heat structure.
 */
class WallTemperatureFromHS3DAux : public AuxKernel
{
public:
  WallTemperatureFromHS3DAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  /// User object to compute the average wall temperature
  const LayeredSideAverage & _T_wall_avg_uo;

public:
  static InputParameters validParams();
};
