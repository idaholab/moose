#pragma once

#include "AuxScalarKernel.h"

class ADShaftConnectedCompressor1PhaseUserObject;

/**
 * Change in pressure computed in the 1-phase shaft-connected compressor
 */
class Compressor1PhaseDeltaPAux : public AuxScalarKernel
{
public:
  Compressor1PhaseDeltaPAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  /// 1-phase shaft-connected compressor user object
  const ADShaftConnectedCompressor1PhaseUserObject & _compressor_uo;

public:
  static InputParameters validParams();
};
