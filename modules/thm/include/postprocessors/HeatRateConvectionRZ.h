#pragma once

#include "SideIntegralPostprocessor.h"
#include "RZSymmetry.h"

class HeatRateConvectionRZ;

template <>
InputParameters validParams<HeatRateConvectionRZ>();

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
  /// Ambient temperature
  const Real & _T_ambient;
  /// Heat transfer coefficient
  const Real & _htc;
};
