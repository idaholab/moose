#pragma once

#include "SideIntegralPostprocessor.h"
#include "RZSymmetry.h"

/**
 * Integrates a cylindrical heat structure boundary convective heat flux
 */
class HeatRateConvectionRZ : public SideIntegralPostprocessor, public RZSymmetry
{
public:
  HeatRateConvectionRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Temperature
  const VariableValue & _T;
  /// Ambient temperature function
  const Function & _T_ambient_fn;
  /// Ambient heat transfer coefficient function
  const Function & _htc_ambient_fn;

public:
  static InputParameters validParams();
};
