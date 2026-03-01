#pragma once

#include "AuxKernel.h"

class HenryGasConstantAux : public AuxKernel
{
public:
  static InputParameters validParams();

  HenryGasConstantAux(const InputParameters & parameters);

  virtual ~HenryGasConstantAux() {}

  /// multiplier to convert atm to Pa
  static constexpr Real _atm_to_Pa = 101325;

  /// Universal gas constant [J/mol/K]
  static constexpr Real _Rgas = 8.31446261815324;

  /// Model constants obtained from
  /// K. Lee, et al., "Semi-empirical model for Henry's law constant of noble gases in molten salt",
  /// Scientific Reports (2024) 14:12847, https://doi.org/10.1038/s41598-024-60006-9.

  /// Model constants for FLiBe
  static constexpr Real _alpha_FLiBe = -3.3584;
  static constexpr Real _beta_FLiBe = -0.0215;
  static constexpr Real _KH0_FLiBe = 7.8622e-1 / _atm_to_Pa;
  static constexpr Real _gamma_0_FLiBe = 235.5;
  static constexpr Real _dgamma_dT_FLiBe = -0.09;

  /// Model constants for FLiNaK
  static constexpr Real _alpha_FLiNaK = -4.6541;
  static constexpr Real _beta_FLiNaK = 0.0105;
  static constexpr Real _KH0_FLiNaK = 1.4246 / _atm_to_Pa;
  static constexpr Real _gamma_0_FLiNaK = 237.0;
  static constexpr Real _dgamma_dT_FLiNaK = -0.0788;

protected:
  virtual Real computeValue();

  /// fluid temperature
  const VariableValue & _temperature;

  /// van der Waals radius
  const Real _radius;

  /// Enum used to select the salt type
  const enum class Saltlist { FLIBE, FLINAK, CUSTOM } _salt_list;

  /// Fit coefficients for the model
  Real _alpha;
  Real _beta;
  Real _gamma_0;
  Real _dgamma_dT;
  Real _KH0;
};
