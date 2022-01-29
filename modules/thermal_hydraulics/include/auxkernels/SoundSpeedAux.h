#pragma once

#include "AuxKernel.h"

class SinglePhaseFluidProperties;

/**
 * Computes the sound speed, given the equation of state
 */
class SoundSpeedAux : public AuxKernel
{
public:
  SoundSpeedAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  const VariableValue & _v;
  const VariableValue & _e;

  const SinglePhaseFluidProperties & _fp;

public:
  static InputParameters validParams();
};
