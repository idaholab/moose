#pragma once

#include "InitialCondition.h"

class RhoEFromPressureTemperatureVelocityIC;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<RhoEFromPressureTemperatureVelocityIC>();

/**
 * The RhoEFromPressureTemperatureVelocityIC returns:
 *
 * rho * e(p, rho) + 0.5 * rho * velocity * velocity
 *
 */
class RhoEFromPressureTemperatureVelocityIC : public InitialCondition
{
public:
  RhoEFromPressureTemperatureVelocityIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p);

  const SinglePhaseFluidProperties & _fp;
  /// The pressure
  const VariableValue & _p;
  /// The temperature
  const VariableValue & _T;
  /// The velocity
  const VariableValue & _vel;
};
