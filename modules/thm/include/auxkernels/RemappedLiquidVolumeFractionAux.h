#pragma once

#include "AuxKernel.h"

class RemappedLiquidVolumeFractionAux;
class Function;
class VolumeFractionMapper;

template <>
InputParameters validParams<RemappedLiquidVolumeFractionAux>();

/**
 * Computes the remapping of liquid volume fraction from a vapor volume fraction function.
 *
 * This aux is used only if no phase interaction is used.
 */
class RemappedLiquidVolumeFractionAux : public AuxKernel
{
public:
  RemappedLiquidVolumeFractionAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Vapor volume fraction function
  const Function & _alpha_vapor;

  /// Volume fraction mapper
  const VolumeFractionMapper & _vfm;
};
