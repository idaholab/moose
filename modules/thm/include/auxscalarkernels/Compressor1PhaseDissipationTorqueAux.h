#pragma once

#include "AuxScalarKernel.h"

class ADShaftConnectedCompressor1PhaseUserObject;

/**
 * Dissipation torque computed in the 1-phase shaft-connected compressor
 */
class Compressor1PhaseDissipationTorqueAux : public AuxScalarKernel
{
public:
  Compressor1PhaseDissipationTorqueAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  /// 1-phase shaft-connected compressor user object
  const ADShaftConnectedCompressor1PhaseUserObject & _compressor_uo;

public:
  static InputParameters validParams();
};
