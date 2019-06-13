#pragma once

#include "InitialCondition.h"

class RhoEAFromPressureTemperatureVelocityIC;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<RhoEAFromPressureTemperatureVelocityIC>();

/**
 * The RhoEAFromPressureTemperatureVelocityIC returns:
 *
 * rho * e(p, rho) * A + 0.5 * rho * velocity * velocity * A
 *
 */
class RhoEAFromPressureTemperatureVelocityIC : public InitialCondition
{
public:
  RhoEAFromPressureTemperatureVelocityIC(const InputParameters & parameters);

protected:
  virtual Real value(const Point & p);

  const SinglePhaseFluidProperties & _fp;
  /// The pressure
  const VariableValue & _p;
  /// The temperature
  const VariableValue & _T;
  /// The velocity
  const VariableValue & _vel;
  /// Cross-sectional area
  const VariableValue & _area;
};
