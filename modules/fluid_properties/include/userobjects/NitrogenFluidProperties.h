//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "HelmholtzFluidProperties.h"
#include <array>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverloaded-virtual"

/**
 * Nitrogen (N2) fluid properties as a function of pressure (Pa)
 * and temperature (K).
 *
 * Thermodynamic properties calculated from:
 * Span,. Lemmon, Jacobsen, Wagner and Yokozeki, A reference equation of state
 * for the thermodynamic properties of nitrogen for temeperatures from
 * 63.151 to 1000 K and pressures to 2200 MPa, Journal of Physical
 * and Chemical Reference Data, 29, 1361--1433 (2000)
 *
 * Viscosity and thermal conductivity calculated from:
 * Lemmon and Jacobsen, Viscosity and Thermal Conductivity Equations for Nitrogen,
 * Oxygen, Argon, and Air, International Journal of Thermophysics, 25, 21--69 (2004)
 *
 */
class NitrogenFluidProperties : public HelmholtzFluidProperties
{
public:
  static InputParameters validParams();

  NitrogenFluidProperties(const InputParameters & parameters);

  virtual std::string fluidName() const override;

  virtual Real molarMass() const override;

  virtual Real mu_from_rho_T(Real density, Real temperature) const override;

  void mu_from_rho_T(Real density,
                     Real temperature,
                     Real ddensity_dT,
                     Real & mu,
                     Real & dmu_drho,
                     Real & dmu_dT) const;

  virtual Real mu_from_p_T(Real pressure, Real temperature) const override;

  virtual void mu_from_p_T(
      Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const override;

  virtual void
  rho_mu_from_p_T(Real pressure, Real temperature, Real & rho, Real & mu) const override;

  virtual void rho_mu_from_p_T(Real pressure,
                               Real temperature,
                               Real & rho,
                               Real & drho_dp,
                               Real & drho_dT,
                               Real & mu,
                               Real & dmu_dp,
                               Real & dmu_dT) const override;

  virtual Real k_from_rho_T(Real density, Real temperature) const override;

  virtual Real k_from_p_T(Real pressure, Real temperature) const override;

  virtual void
  k_from_p_T(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const override;

  virtual std::vector<Real> henryCoefficients() const override;

  virtual Real criticalPressure() const override;

  virtual Real criticalTemperature() const override;

  virtual Real criticalDensity() const override;

  virtual Real triplePointPressure() const override;

  virtual Real triplePointTemperature() const override;

  virtual Real vaporPressure(Real temperature) const override;

  virtual void vaporPressure(Real temperature, Real & psat, Real & dpsat_dT) const override;

  /**
   * Saturated liquid density of N2
   * Valid for temperatures between the triple point temperature
   * and critical temperature
   *
   * Eq. (5), from Span et al (reference above)
   *
   * @param temperature N2 temperature (K)
   * @return saturated liquid density (kg/m^3)
   */
  Real saturatedLiquidDensity(Real temperature) const;

  /**
   * Saturated vapor density of N2
   * Valid for temperatures between the triple point temperature
   * and critical temperature
   *
   * Eq. (6), from Span et al (reference above)
   *
   * @param temperature N2 temperature (K)
   * @return saturated vapor density (kg/m^3)
   */
  Real saturatedVaporDensity(Real temperature) const;

protected:
  virtual Real alpha(Real delta, Real tau) const override;

  virtual Real dalpha_ddelta(Real delta, Real tau) const override;

  virtual Real dalpha_dtau(Real delta, Real tau) const override;

  virtual Real d2alpha_ddelta2(Real delta, Real tau) const override;

  virtual Real d2alpha_dtau2(Real delta, Real tau) const override;

  virtual Real d2alpha_ddeltatau(Real delta, Real tau) const override;

  /// Nitrogen molar mass (kg/mol)
  const Real _Mn2;
  /// Critical pressure (Pa)
  const Real _p_critical;
  /// Critical temperature (K)
  const Real _T_critical;
  /// Critical molar density (mol/l)
  const Real _rho_molar_critical;
  /// Critical density (kg/m^3)
  const Real _rho_critical;
  /// Triple point pressure (Pa)
  const Real _p_triple;
  /// Triple point temperature (K)
  const Real _T_triple;

  /// Coefficients for ideal gas component of the Helmholtz free energy
  const std::array<Real, 8> _a{{2.5,
                                -12.76952708,
                                -0.00784163,
                                -1.934819e-4,
                                -1.247742e-5,
                                6.678326e-8,
                                1.012941,
                                26.65788}};
  /// Coefficients for residual component of the Helmholtz free energy
  const std::array<Real, 6> _N1{{0.924803575275,
                                 -0.492448489428,
                                 0.661883336938,
                                 -0.192902649201e1,
                                 -0.622469309629e-1,
                                 0.349943957581}};
  const std::array<unsigned int, 6> _i1{{1, 1, 2, 2, 3, 3}};
  const std::array<Real, 6> _j1{{0.25, 0.875, 0.5, 0.875, 0.375, 0.75}};

  const std::array<Real, 26> _N2{
      {0.564857472498,     -0.161720005987e1,  -0.481395031883,    0.421150636384,
       -0.161962230825e-1, 0.172100994165,     0.735448924933e-2,  0.168077305479e-1,
       -0.107626664179e-2, -0.137318088513e-1, 0.635466899859e-3,  0.304432279419e-2,
       -0.435762336045e-1, -0.723174889316e-1, 0.389644315272e-1,  -0.21220136391e-1,
       0.4808822981509e-2, -0.551990017984e-4, -0.462016716479e-1, -0.300311716011e-2,
       0.368825891208e-1,  -0.25585684622e-2,  0.896915264558e-2,  -0.44151337035e-2,
       0.133722924858e-2,  0.264832491957e-3}};
  const std::array<unsigned int, 26> _i2{
      {1, 1, 1, 3, 3, 4, 6, 6, 7, 7, 8, 8, 1, 2, 3, 4, 5, 8, 4, 5, 5, 8, 3, 5, 6, 9}};
  const std::array<Real, 26> _j2{{0.5,  0.75, 2.0,  1.25, 3.5,  1.0, 0.5, 3.0, 0.0,
                                  2.75, 0.75, 2.5,  4.0,  6.0,  6.0, 3.0, 3.0, 6.0,
                                  16.0, 11.0, 15.0, 12.0, 12.0, 7.0, 4.0, 16.0}};
  const std::array<unsigned int, 26> _l2{
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4}};

  const std::array<Real, 4> _N3{
      {0.196688194015e2, -0.20911560073e2, 0.167788306989e-1, 0.262767566274e4}};
  const std::array<unsigned int, 4> _i3{{1, 1, 3, 2}};
  const std::array<unsigned int, 4> _j3{{0, 1, 2, 3}};
  const std::array<unsigned int, 4> _l3{{2, 2, 2, 2}};
  const std::array<Real, 4> _phi3{{20.0, 20.0, 15.0, 25.0}};
  const std::array<Real, 4> _beta3{{325.0, 325.0, 300.0, 275.0}};
  const std::array<Real, 4> _gamma3{{1.16, 1.16, 1.13, 1.25}};

  /// Coefficients for viscosity
  const std::array<Real, 5> _bmu{{0.431, -0.4623, 0.08406, 0.005341, -0.00331}};
  const std::array<Real, 5> _Nmu{{10.72, 0.03989, 0.001208, -7.402, 4.62}};
  const std::array<Real, 5> _tmu{{0.1, 0.25, 3.2, 0.9, 0.3}};
  const std::array<Real, 5> _dmu{{2, 10, 12, 2, 1}};
  const std::array<Real, 5> _lmu{{0, 1, 1, 2, 3}};
  const std::array<Real, 5> _gammamu{{0.0, 1.0, 1.0, 1.0, 1.0}};

  /// Coefficients for thermal conductivity
  const std::array<Real, 6> _Nk{{8.862, 31.11, -73.13, 20.03, -0.7096, 0.2672}};
  const std::array<Real, 6> _tk{{0.0, 0.03, 0.2, 0.8, 0.6, 1.9}};
  const std::array<unsigned int, 6> _dk{{1, 2, 3, 4, 8, 10}};
  const std::array<unsigned int, 6> _lk{{0, 0, 1, 2, 2, 2}};
  const std::array<Real, 6> _gammak{{0.0, 0.0, 1.0, 1.0, 1.0}};
};

#pragma GCC diagnostic pop
