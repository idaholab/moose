#ifndef RHOVAPORMIXTUREFROMPRESSURETEMPERATUREIC_H
#define RHOVAPORMIXTUREFROMPRESSURETEMPERATUREIC_H

#include "InitialCondition.h"
#include "VaporMixtureInterface.h"

class RhoVaporMixtureFromPressureTemperatureIC;

template <>
InputParameters validParams<RhoVaporMixtureFromPressureTemperatureIC>();

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
};

#endif // RHOVAPORMIXTUREFROMPRESSURETEMPERATUREIC_H
