#pragma once

#include "SideIntegralPostprocessor.h"
#include "RZSymmetry.h"

class HeatRateRadiationRZ;

template <>
InputParameters validParams<HeatRateRadiationRZ>();

/**
 * Integrates a cylindrical heat structure boundary radiative heat flux
 */
class HeatRateRadiationRZ : public SideIntegralPostprocessor, public RZSymmetry
{
public:
  HeatRateRadiationRZ(const InputParameters & parameters);

protected:
  virtual Real computeQpIntegral() override;

  /// Temperature
  const VariableValue & _T;
  /// Ambient temperature
  const Function & _T_ambient;
  /// Emissivity
  const Real & _emissivity;
  /// View factor function
  const Function & _view_factor_fn;
  /// Stefan-Boltzmann constant
  const Real & _sigma_stefan_boltzmann;
};
