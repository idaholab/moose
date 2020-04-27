#pragma once

#include "ConstantScalarAux.h"

class SinglePhaseFluidProperties;

/**
 * Computes turbine power for 1-phase flow
 */
class TurbinePower1PhaseAux : public ConstantScalarAux
{
public:
  TurbinePower1PhaseAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Flag indicating if turbine is operating or not
  const bool & _on;

public:
  static InputParameters validParams();
};
