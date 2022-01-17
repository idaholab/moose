#pragma once

#include "AuxKernel.h"

class SinglePhaseFluidProperties;

/**
 * Computes convective heat flux for 1-phase flow
 */
class ConvectiveHeatFlux1PhaseAux : public AuxKernel
{
public:
  ConvectiveHeatFlux1PhaseAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Wall temperature
  const VariableValue & _T_wall;
  /// Fluid temperature
  const MaterialProperty<Real> & _T;
  /// Wall heat transfer coefficient
  const MaterialProperty<Real> & _Hw;
  /// Scaling factor
  const Real & _scaling_factor;

public:
  static InputParameters validParams();
};
