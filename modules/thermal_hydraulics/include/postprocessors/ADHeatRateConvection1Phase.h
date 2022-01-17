#pragma once

#include "ElementIntegralPostprocessor.h"

/**
 * Computes convective heat rate into a 1-phase flow channel
 */
class ADHeatRateConvection1Phase : public ElementIntegralPostprocessor
{
public:
  ADHeatRateConvection1Phase(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral();

  /// Wall temperature
  const ADMaterialProperty<Real> & _T_wall;
  /// Fluid temperature
  const ADMaterialProperty<Real> & _T;
  /// Convective heat transfer coefficient, W/m^2-K
  const ADMaterialProperty<Real> & _Hw;
  /// Heat flux perimeter
  const ADVariableValue & _P_hf;

public:
  static InputParameters validParams();
};
