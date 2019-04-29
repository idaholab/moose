#pragma once

#include "Kernel.h"
#include "Function.h"

class OneDHeatForcingFunction;

template <>
InputParameters validParams<OneDHeatForcingFunction>();

/**
 *
 */
class OneDHeatForcingFunction : public Kernel
{
public:
  OneDHeatForcingFunction(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  const Real & _power_fraction;
  const VariableValue & _total_power;
  const Real & _volume;
  Function & _power_shape_function;
};
