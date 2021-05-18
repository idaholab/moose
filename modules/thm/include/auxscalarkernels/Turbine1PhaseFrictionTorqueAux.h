#pragma once

#include "AuxScalarKernel.h"

class ShaftConnectedTurbine1PhaseUserObject;

/**
 * Friction torque computed in the 1-phase shaft-connected turbine
 */
class Turbine1PhaseFrictionTorqueAux : public AuxScalarKernel
{
public:
  Turbine1PhaseFrictionTorqueAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  /// 1-phase shaft-connected turbine user object
  const ShaftConnectedTurbine1PhaseUserObject & _turbine_uo;

public:
  static InputParameters validParams();
};
