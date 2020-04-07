#pragma once

#include "AuxKernel.h"

class SinglePhaseFluidProperties;

/**
 * Computes Mach number
 */
class MachNumberAux : public AuxKernel
{
public:
  MachNumberAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _vel;
  const VariableValue & _v;
  const VariableValue & _e;

  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
