#pragma once

#include "AuxScalarKernel.h"

class ADShaftConnectedTurbine1PhaseUserObject;

/**
 * Moment of inertia computed in the 1-phase shaft-connected turbine
 */
class Turbine1PhaseMomentOfInertiaAux : public AuxScalarKernel
{
public:
  Turbine1PhaseMomentOfInertiaAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  /// 1-phase shaft-connected turbine user object
  const ADShaftConnectedTurbine1PhaseUserObject & _turbine_uo;

public:
  static InputParameters validParams();
};
