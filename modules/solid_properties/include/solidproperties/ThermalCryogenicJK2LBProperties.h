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
 * Thermal properties of JK2LB austenitic stainless steel for cryogenic applications.
 *
 * JK2LB is a Japanese-grade cryogenic austenitic stainless steel used as the structural
 * jacket in ITER superconducting cable assemblies. Properties are temperature-dependent
 * correlations fitted to experimental data from the literature.
 *
 * Data source: https://www.sciencedirect.com/science/article/pii/S0011227508001835
 * Valid temperature range: 2-300 K
 *
 * Thermal conductivity and specific heat use log-polynomial correlations fitted via
 * nonlinear least squares (scipy.optimize.curve_fit). The functional form matches
 * NIST patterns used for cryogenic metals (e.g., OFHC copper) and 304 stainless steel.
 */
class ThermalCryogenicJK2LBProperties : public ThermalSolidProperties
{
public:
  static InputParameters validParams();

  ThermalCryogenicJK2LBProperties(const InputParameters & parameters);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

  virtual Real k_from_T(const Real & T) const override;
  virtual void k_from_T(const Real & T, Real & k, Real & dk_dT) const override;

  virtual Real cp_from_T(const Real & T) const override;
  virtual void cp_from_T(const Real & T, Real & cp, Real & dcp_dT) const override;

  virtual Real rho_from_T(const Real & T) const override;
  virtual void rho_from_T(const Real & T, Real & rho, Real & drho_dT) const override;

  virtual Real cp_integral(const Real & T) const override;

#pragma GCC diagnostic pop

protected:
  /// Valid temperature range [K]
  const Real _T_min;
  const Real _T_max;

  /// Density (constant) [kg/m^3]
  const Real _rho_const;

  /// Number of intervals for numerical integration of cp
  const unsigned int _cp_integral_n_intervals;

  /// Thermal conductivity correlation coefficients
  /// log10(k) = c0 + c1*log10(T) + c2*log10(T)^2 + c3*log10(T)^3 + c4*log10(T)^4
  /// Fitted to digitized data via scipy.optimize.curve_fit (R^2 = 0.999832)
  const Real _k_c0;
  const Real _k_c1;
  const Real _k_c2;
  const Real _k_c3;
  const Real _k_c4;

  /// Specific heat correlation coefficients
  /// log10(cp) = c0 + c1*log10(T) + ... + c7*log10(T)^7
  /// Fitted to digitized data via scipy.optimize.curve_fit (R^2 = 0.999898)
  const Real _cp_c0;
  const Real _cp_c1;
  const Real _cp_c2;
  const Real _cp_c3;
  const Real _cp_c4;
  const Real _cp_c5;
  const Real _cp_c6;
  const Real _cp_c7;
};
