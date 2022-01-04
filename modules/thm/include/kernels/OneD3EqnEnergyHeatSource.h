#pragma once

#include "Kernel.h"

class Function;

/**
 * Volumetric heat source for 1-phase flow channel
 */
class OneD3EqnEnergyHeatSource : public Kernel
{
public:
  OneD3EqnEnergyHeatSource(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual();
  virtual Real computeQpJacobian();
  virtual Real computeQpOffDiagJacobian(unsigned int jvar);

  /// Heat source function
  const Function & _q;
  /// Cross sectional area
  const VariableValue & _A;

public:
  static InputParameters validParams();
};
