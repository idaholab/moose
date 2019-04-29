#pragma once

#include "AuxKernel.h"

class ReynoldsNumberAux;
class SinglePhaseFluidProperties;

template <>
InputParameters validParams<ReynoldsNumberAux>();

/**
 * Computes Reynolds number
 */
class ReynoldsNumberAux : public AuxKernel
{
public:
  ReynoldsNumberAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Volume fraction
  const VariableValue & _alpha;
  /// Density of the phase
  const VariableValue & _rho;
  /// Velocity of the phase
  const VariableValue & _vel;
  /// Hydraulic diameter
  const VariableValue & _D_h;
  /// Specific volume
  const VariableValue & _v;
  /// Specific internal energy
  const VariableValue & _e;

  const SinglePhaseFluidProperties & _fp;
};
