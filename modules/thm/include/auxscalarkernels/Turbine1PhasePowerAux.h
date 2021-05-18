#pragma once

#include "AuxScalarKernel.h"

class ShaftConnectedTurbine1PhaseUserObject;

/**
 * Power extracted from flow computed in the 1-phase shaft-connected turbine
 */
class Turbine1PhasePowerAux : public AuxScalarKernel
{
public:
  Turbine1PhasePowerAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  /// 1-phase shaft-connected turbine user object
  const ShaftConnectedTurbine1PhaseUserObject & _turbine_uo;

public:
  static InputParameters validParams();
};
