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
class ThermalCopperProperties : public ThermalSolidProperties
{
public:
  static InputParameters validParams();

  ThermalCopperProperties(const InputParameters & parameters);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

  virtual Real k_from_T(const Real & T) const override;

  virtual void k_from_T(const Real & T, Real & k, Real & dk_dT) const override;

  virtual Real cp_from_T(const Real & T) const override;

  virtual void cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const override;

  virtual Real rho_from_T(const Real & T) const override;

  virtual void rho_from_T(const Real & T, Real & rho, Real & drho_dT) const override;

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

  /// Constant density [kg/m³]
  const Real & _rho_const;

  /// Thermal conductivity coefficients for RRR = 50
  const Real _k50_a, _k50_b, _k50_c, _k50_d, _k50_e, _k50_f, _k50_g, _k50_h, _k50_i;

  /// Thermal conductivity coefficients for RRR = 100
  const Real _k100_a, _k100_b, _k100_c, _k100_d, _k100_e, _k100_f, _k100_g, _k100_h, _k100_i;

  /// Thermal conductivity coefficients for RRR = 150
  const Real _k150_a, _k150_b, _k150_c, _k150_d, _k150_e, _k150_f, _k150_g, _k150_h, _k150_i;

  /// Thermal conductivity coefficients for RRR = 300
  const Real _k300_a, _k300_b, _k300_c, _k300_d, _k300_e, _k300_f, _k300_g, _k300_h, _k300_i;

  /// Thermal conductivity coefficients for RRR = 500
  const Real _k500_a, _k500_b, _k500_c, _k500_d, _k500_e, _k500_f, _k500_g, _k500_h, _k500_i;

  /// Specific heat coefficients (RRR-independent)
  const Real _cp_a, _cp_b, _cp_c, _cp_d, _cp_e, _cp_f, _cp_g, _cp_h;

private:
  /**
   * Helper function to compute thermal conductivity from NIST correlation
   * log10(k) = (a + c*T^0.5 + e*T + g*T^1.5 + i*T^2) / (1 + b*T^0.5 + d*T + f*T^1.5 + h*T^2)
   * k [W/(m·K)], T [K]
   */
  void computeThermalConductivity(const Real & T,
                                  const Real & a,
                                  const Real & b,
                                  const Real & c,
                                  const Real & d,
                                  const Real & e,
                                  const Real & f,
                                  const Real & g,
                                  const Real & h,
                                  const Real & i,
                                  Real & k,
                                  Real & dk_dT) const;
};

#pragma GCC diagnostic pop
