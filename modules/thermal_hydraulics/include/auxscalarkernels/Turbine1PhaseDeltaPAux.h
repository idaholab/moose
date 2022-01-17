#pragma once

#include "AuxScalarKernel.h"

class ADShaftConnectedTurbine1PhaseUserObject;

/**
 * Change in pressure computed in the 1-phase shaft-connected turbine
 */
class Turbine1PhaseDeltaPAux : public AuxScalarKernel
{
public:
  Turbine1PhaseDeltaPAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  /// 1-phase shaft-connected turbine user object
  const ADShaftConnectedTurbine1PhaseUserObject & _turbine_uo;

public:
  static InputParameters validParams();
};
