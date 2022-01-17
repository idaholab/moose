#pragma once

#include "HeatRateConvection.h"
#include "RZSymmetry.h"

/**
 * Integrates a cylindrical heat structure boundary convective heat flux
 */
class HeatRateConvectionRZ : public HeatRateConvection, public RZSymmetry
{
public:
  HeatRateConvectionRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

public:
  static InputParameters validParams();
};
