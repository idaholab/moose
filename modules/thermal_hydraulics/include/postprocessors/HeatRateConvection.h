#pragma once

#include "SideIntegralPostprocessor.h"

/**
 * Integrates a convective heat flux over a boundary.
 */
class HeatRateConvection : public SideIntegralPostprocessor
{
public:
  HeatRateConvection(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Temperature
  const VariableValue & _T;
  /// Ambient temperature function
  const Function & _T_ambient_fn;
  /// Ambient heat transfer coefficient function
  const Function & _htc_ambient_fn;
  /// Factor by which to scale integral, like when using a 2D domain
  const Real & _scale;

public:
  static InputParameters validParams();
};
