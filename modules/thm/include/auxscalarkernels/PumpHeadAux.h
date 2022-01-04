#pragma once

#include "AuxScalarKernel.h"

class ADShaftConnectedPump1PhaseUserObject;

/**
 * Head computed in the 1-phase shaft-connected pump
 */
class PumpHeadAux : public AuxScalarKernel
{
public:
  PumpHeadAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  /// 1-phase shaft-connected pump user object
  const ADShaftConnectedPump1PhaseUserObject & _pump_uo;

public:
  static InputParameters validParams();
};
