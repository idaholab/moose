#pragma once

#include "AuxScalarKernel.h"

class ShaftConnectedCompressor1PhaseUserObject;

/**
 * Friction torque computed in the 1-phase shaft-connected compressor
 */
class Compressor1PhaseFrictionAux : public AuxScalarKernel
{
public:
  Compressor1PhaseFrictionAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  /// 1-phase shaft-connected compressor user object
  const ShaftConnectedCompressor1PhaseUserObject & _compressor_uo;

public:
  static InputParameters validParams();
};
