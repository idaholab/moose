#pragma once

#include "HeatRateRadiation.h"
#include "RZSymmetry.h"

/**
 * Integrates a cylindrical heat structure boundary radiative heat flux
 */
class HeatRateRadiationRZ : public HeatRateRadiation, public RZSymmetry
{
public:
  HeatRateRadiationRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

public:
  static InputParameters validParams();
};
