#pragma once

#include "AuxKernel.h"

/**
 * Computes specific volume
 */
class THMSpecificVolumeAux : public AuxKernel
{
public:
  THMSpecificVolumeAux(const InputParameters & parameters);

protected:
  Real computeValue();

  const VariableValue & _rhoA;
  const VariableValue & _area;
  const VariableValue & _alpha;

public:
  static InputParameters validParams();
};
