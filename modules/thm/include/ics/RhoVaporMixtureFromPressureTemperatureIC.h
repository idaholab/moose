#pragma once

#include "InitialCondition.h"
#include "VaporMixtureInterface.h"

/**
 * Computes the density of a vapor mixture from pressure and temperature.
 */
class RhoVaporMixtureFromPressureTemperatureIC : public VaporMixtureInterface<InitialCondition>
{
public:
  RhoVaporMixtureFromPressureTemperatureIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p);

  /// Pressure
  const VariableValue & _p;
  /// Temperature
  const VariableValue & _T;

public:
  static InputParameters validParams();
};
