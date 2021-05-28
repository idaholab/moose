#pragma once

#include "InitialCondition.h"

class SinglePhaseFluidProperties;

/**
 * Computes initial viscosity from specified pressure and temperature
 */
class ViscosityIC : public InitialCondition
{
public:
  static InputParameters validParams();

  ViscosityIC(const InputParameters & parameters);

  virtual Real value(const Point & p) override;

protected:
  /// Temperature
  const VariableValue & _T;
  /// Pressure
  const Real & _P;
  /// Fluid properties
  const SinglePhaseFluidProperties & _fp;
};
