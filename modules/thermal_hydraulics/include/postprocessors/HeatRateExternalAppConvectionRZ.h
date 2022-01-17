#pragma once

#include "SideIntegralPostprocessor.h"
#include "RZSymmetry.h"

/**
 * Integrates a cylindrical heat structure boundary convective heat flux from an external
 * application
 */
class HeatRateExternalAppConvectionRZ : public SideIntegralPostprocessor, public RZSymmetry
{
public:
  HeatRateExternalAppConvectionRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Temperature
  const VariableValue & _T;
  /// Temperature from external application
  const VariableValue & _T_ext;
  /// Heat transfer coefficient from external application
  const VariableValue & _htc_ext;

public:
  static InputParameters validParams();
};
