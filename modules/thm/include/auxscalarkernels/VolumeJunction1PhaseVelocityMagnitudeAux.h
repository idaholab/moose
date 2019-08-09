#pragma once

#include "AuxScalarKernel.h"

class VolumeJunction1PhaseVelocityMagnitudeAux;

template <>
InputParameters validParams<VolumeJunction1PhaseVelocityMagnitudeAux>();

/**
 * Computes magnitude of velocity from the 1-phase volume junction variables
 */
class VolumeJunction1PhaseVelocityMagnitudeAux : public AuxScalarKernel
{
public:
  VolumeJunction1PhaseVelocityMagnitudeAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// rho*V of the junction
  const VariableValue & _rhoV;
  /// rho*u*V of the junction
  const VariableValue & _rhouV;
  /// rho*v*V of the junction
  const VariableValue & _rhovV;
  /// rho*w*V of the junction
  const VariableValue & _rhowV;
};
