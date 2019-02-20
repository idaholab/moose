#ifndef RHOEVAPORMIXTUREFROMPRESSURETEMPERATUREVELOCITYIC_H
#define RHOEVAPORMIXTUREFROMPRESSURETEMPERATUREVELOCITYIC_H

#include "InitialCondition.h"
#include "VaporMixtureInterface.h"

class RhoEVaporMixtureFromPressureTemperatureVelocityIC;

template <>
InputParameters validParams<RhoEVaporMixtureFromPressureTemperatureVelocityIC>();

/**
 * Computes the total energy density of a vapor mixture from pressure, temperature, and velocity.
 */
class RhoEVaporMixtureFromPressureTemperatureVelocityIC
  : public VaporMixtureInterface<InitialCondition>
{
public:
  RhoEVaporMixtureFromPressureTemperatureVelocityIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p);

  /// Pressure
  const VariableValue & _p;
  /// Temperature
  const VariableValue & _T;
  /// Velocity
  const VariableValue & _vel;
};

#endif // RHOEVAPORMIXTUREFROMPRESSURETEMPERATUREVELOCITYIC_H
