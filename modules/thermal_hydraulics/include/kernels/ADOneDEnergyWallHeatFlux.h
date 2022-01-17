#pragma once

#include "ADKernel.h"

class ADOneDEnergyWallHeatFlux : public ADKernel
{
public:
  ADOneDEnergyWallHeatFlux(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  /// Heat flux
  const ADMaterialProperty<Real> & _q_wall;
  /// Heat flux perimeter
  const ADVariableValue & _P_hf;

public:
  static InputParameters validParams();
};
