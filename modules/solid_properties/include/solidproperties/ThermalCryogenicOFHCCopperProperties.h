//* This file is part of the MOOSE framework
//* https://mooseframework.inl.gov
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ThermalSolidProperties.h"

/**
 * Oxygen-free high-conductivity (OFHC) copper thermal properties as a function of temperature.
 * Data from NIST Cryogenic Materials Database for UNS C10100/C10200.
 * Valid range: 4-300 K.
 */
class ThermalCryogenicOFHCCopperProperties : public ThermalSolidProperties
{
public:
  static InputParameters validParams();

  ThermalCryogenicOFHCCopperProperties(const InputParameters & parameters);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

  virtual Real k_from_T(const Real & T) const override;

  virtual void k_from_T(const Real & T, Real & k, Real & dk_dT) const override;

  virtual Real cp_from_T(const Real & T) const override;

  virtual void cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const override;

  virtual Real rho_from_T(const Real & T) const override;

  virtual void rho_from_T(const Real & T, Real & rho, Real & drho_dT) const override;

  virtual Real cp_integral(const Real & T) const override;

protected:
  /// Enumeration for selecting the residual resistivity ratio (RRR)
  enum RRRValue
  {
    RRR_50,
    RRR_100,
    RRR_150,
    RRR_300,
    RRR_500
  } _rrr;

  /// Integer index corresponding to _rrr for array access
  const unsigned int _rrr_index;

  /// Constant density [kg/m^3]
  const Real & _rho_const;

  /// Number of intervals for cp_integral numerical integration
  const unsigned int _cp_integral_n_intervals;

  /// Static arrays of thermal conductivity coefficients for each RRR value
  /// Indices: [RRR_50, RRR_100, RRR_150, RRR_300, RRR_500]
  static constexpr Real _ka_values[5] = {1.8743, 2.2154, 2.3797, 1.357, 2.8075};
  static constexpr Real _kb_values[5] = {-0.41538, -0.47461, -0.4918, 0.3981, -0.54074};
  static constexpr Real _kc_values[5] = {-0.6018, -0.88068, -0.98615, 2.669, -1.2777};
  static constexpr Real _kd_values[5] = {0.13294, 0.13871, 0.13942, -0.1346, 0.15362};
  static constexpr Real _ke_values[5] = {0.26426, 0.29505, 0.30475, -0.6683, 0.36444};
  static constexpr Real _kf_values[5] = {-0.0219, -0.02043, -0.019713, 0.01342, -0.02105};
  static constexpr Real _kg_values[5] = {-0.051276, -0.04831, -0.046897, 0.05773, -0.051727};
  static constexpr Real _kh_values[5] = {0.0014871, 0.001281, 0.0011969, 0.0002147, 0.0012226};
  static constexpr Real _ki_values[5] = {0.003723, 0.003207, 0.0029988, 0.0, 0.0030964};

  /// Thermal conductivity coefficients for this instance's RRR
  const Real _k_a, _k_b, _k_c, _k_d, _k_e, _k_f, _k_g, _k_h, _k_i;

  /// Specific heat coefficients (RRR-independent)
  static constexpr Real _cp_coeff[8] = {
      -1.91844, -0.15973, 8.61013, -18.996, 21.9661, -12.7328, 3.54322, -0.3797};

private:
  /**
   * Helper function to compute thermal conductivity from NIST correlation
   * log10(k) = (a + c*T^0.5 + e*T + g*T^1.5 + i*T^2) / (1 + b*T^0.5 + d*T + f*T^1.5 + h*T^2)
   * k [W/(m K)], T [K]
   */
  void computeThermalConductivity(Real T,
                                  Real a,
                                  Real b,
                                  Real c,
                                  Real d,
                                  Real e,
                                  Real f,
                                  Real g,
                                  Real h,
                                  Real i,
                                  Real & k,
                                  Real & dk_dT) const;
};

#pragma GCC diagnostic pop
