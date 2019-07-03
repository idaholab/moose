#pragma once

#include "AuxScalarKernel.h"

class VolumeJunction1PhaseTemperatureAux;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<VolumeJunction1PhaseTemperatureAux>();

/**
 * Computes temperature from the 1-phase volume junction variables
 */
class VolumeJunction1PhaseTemperatureAux : public AuxScalarKernel
{
public:
  VolumeJunction1PhaseTemperatureAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Volume of the junction
  const Real & _volume;
  /// rho*V of the junction
  const VariableValue & _rhoV;
  /// rho*u*V of the junction
  const VariableValue & _rhouV;
  /// rho*v*V of the junction
  const VariableValue & _rhovV;
  /// rho*w*V of the junction
  const VariableValue & _rhowV;
  /// rho*E*V of the junction
  const VariableValue & _rhoEV;
  /// Single-phase fluid properties user object
  const SinglePhaseFluidProperties & _fp;
};
