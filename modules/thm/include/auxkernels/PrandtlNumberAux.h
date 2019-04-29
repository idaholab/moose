#pragma once

#include "AuxKernel.h"

class PrandtlNumberAux;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<PrandtlNumberAux>();

/**
 * Computes Prandtl number
 */
class PrandtlNumberAux : public AuxKernel
{
public:
  PrandtlNumberAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Specific volume
  const VariableValue & _v;
  /// Specific internal energy
  const VariableValue & _e;

  const SinglePhaseFluidProperties & _fp;
};
