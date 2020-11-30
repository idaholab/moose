#pragma once

#include "AuxScalarKernel.h"

class ShaftConnectedPump1PhaseUserObject;

/**
 * Moment of inertia computed in the 1-phase shaft-connected pump
 */
class PumpInertiaAux : public AuxScalarKernel
{
public:
  PumpInertiaAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  /// 1-phase shaft-connected pump user object
  const ShaftConnectedPump1PhaseUserObject & _pump_uo;

public:
  static InputParameters validParams();
};
