#pragma once

#include "InitialCondition.h"

class SinglePhaseFluidProperties;

/**
 * Computes density from pressure and temperature
 */
class RhoFromPressureTemperatureIC : public InitialCondition
{
public:
  RhoFromPressureTemperatureIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p);

  const SinglePhaseFluidProperties & _spfp;
  /// The pressure
  const VariableValue & _p;
  /// The temperature
  const VariableValue & _T;

public:
  static InputParameters validParams();
};
