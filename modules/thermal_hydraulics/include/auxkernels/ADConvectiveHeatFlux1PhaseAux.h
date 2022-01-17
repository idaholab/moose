#pragma once

#include "AuxKernel.h"

class SinglePhaseFluidProperties;

/**
 * Computes convective heat flux for 1-phase flow
 */
class ADConvectiveHeatFlux1PhaseAux : public AuxKernel
{
public:
  ADConvectiveHeatFlux1PhaseAux(const InputParameters & parameters);

protected:
  virtual Real computeValue();

  /// Wall temperature
  const VariableValue & _T_wall;
  /// Fluid temperature
  const ADMaterialProperty<Real> & _T;
  /// Wall heat transfer coefficient
  const ADMaterialProperty<Real> & _Hw;
  /// Scaling factor
  const Real & _scaling_factor;

public:
  static InputParameters validParams();
};
