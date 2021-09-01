#pragma once

#include "AuxScalarKernel.h"

class ADShaftConnectedTurbine1PhaseUserObject;

/**
 * Flow coefficient computed in the 1-phase shaft-connected turbine
 */
class Turbine1PhaseFlowCoefficientAux : public AuxScalarKernel
{
public:
  Turbine1PhaseFlowCoefficientAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  /// 1-phase shaft-connected turbine user object
  const ADShaftConnectedTurbine1PhaseUserObject & _turbine_uo;

public:
  static InputParameters validParams();
};
