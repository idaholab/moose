#pragma once

#include "ADKernel.h"

// The spatial part of the 1D energy conservation for Navier-Stokes flow
class ADOneDEnergyWallHeating : public ADKernel
{
public:
  ADOneDEnergyWallHeating(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual();

  const ADMaterialProperty<Real> & _temperature;
  const ADMaterialProperty<Real> & _Hw;

  const ADVariableValue & _T_wall;
  const ADVariableValue & _P_hf;

public:
  static InputParameters validParams();
};
