#pragma once

#include "Kernel.h"
#include "Function.h"

/**
 *
 */
class HeatStructureHeatSource : public Kernel
{
public:
  HeatStructureHeatSource(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();

  const Real & _power_fraction;
  const VariableValue & _total_power;
  const Function & _power_shape_function;
  const PostprocessorValue & _power_shape_integral;
  const Real & _scale;
  const Real & _num_units;

public:
  static InputParameters validParams();
};
