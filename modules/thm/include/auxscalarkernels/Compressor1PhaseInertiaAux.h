#pragma once

#include "AuxScalarKernel.h"

class ShaftConnectedCompressor1PhaseUserObject;

/**
 * Moment of inertia computed in the 1-phase shaft-connected compressor
 */
class Compressor1PhaseInertiaAux : public AuxScalarKernel
{
public:
  Compressor1PhaseInertiaAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();
  /// 1-phase shaft-connected compressor user object
  const ShaftConnectedCompressor1PhaseUserObject & _compressor_uo;

public:
  static InputParameters validParams();
};
