#pragma once

#include "SideIntegralPostprocessor.h"

/**
 * Integrates a radiative heat flux over a boundary.
 */
class HeatRateRadiation : public SideIntegralPostprocessor
{
public:
  HeatRateRadiation(const InputParameters & parameters);

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
  /// Factor by which to scale integral, like when using a 2D domain
  const Real & _scale;

public:
  static InputParameters validParams();
};
