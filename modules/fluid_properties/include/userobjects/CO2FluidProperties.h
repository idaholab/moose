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
 * CO2 fluid properties
 * Most thermophysical properties taken from:
 * Span and Wagner, "A New Equation of State for Carbon Dioxide Covering the Fluid Region
 * from the Triple-Point Temperature to 1100K at Pressures up to 800 MPa",
 * J. Phys. Chem. Ref. Data, 25 (1996)
 *
 * Note: the Span and Wagner EOS uses density and temperature as the primary variables. As
 * a result, density must first be found using iteration, after which the other properties
 * can be calculated directly.
 *
 * Viscosity from:
 * Fenghour et al., The viscosity of carbon dioxide, J. Phys. Chem. Ref.Data,
 * 27, 31-44 (1998)
 * Note: critical enhancement not included
 * Valid for 217 K < T < 1000K and rho < 1400 kg/m^3
 *
 * Thermal conductivity from:
 * Scalabrin et al., A Reference Multiparameter Thermal Conductivity
 * Equation for Carbon Dioxide with an Optimized Functional Form, J. Phys.
 * Chem. Ref. Data 35 (2006)
 */
class CO2FluidProperties : public HelmholtzFluidProperties
{
public:
  static InputParameters validParams();

  CO2FluidProperties(const InputParameters & parameters);
  virtual ~CO2FluidProperties();

  virtual Real rho_from_p_T(Real pressure, Real temperature) const override;

  virtual void rho_from_p_T(
      Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const override;

  virtual Real mu_from_p_T(Real pressure, Real temperature) const override;

  virtual void mu_from_p_T(
      Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const override;

  virtual Real mu_from_rho_T(Real density, Real temperature) const override;

  void mu_from_rho_T(Real density,
                     Real temperature,
                     Real ddensity_dT,
                     Real & mu,
                     Real & dmu_drho,
                     Real & dmu_dT) const;

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

  virtual std::string fluidName() const override;

  virtual Real molarMass() const override;

  virtual Real criticalPressure() const override;

  virtual Real criticalTemperature() const override;

  virtual Real criticalDensity() const override;

  virtual Real triplePointPressure() const override;

  virtual Real triplePointTemperature() const override;

  /**
   * Melting pressure. Used to delineate solid and liquid phases
   * Valid for temperatures greater than the triple point temperature
   *
   * Eq. 3.10, from Span and Wagner (reference above)
   *
   * @param temperature CO2 temperature (K)
   * @return melting pressure (Pa)
   */
  Real meltingPressure(Real temperature) const;

  /**
   * Sublimation pressure. Used to delineate solid and gas phases
   * Valid for temperatures less than the triple point temperature
   *
   * Eq. 3.12, from Span and Wagner (reference above)
   *
   * @param temperature CO2 temperature (K)
   * @return sublimation pressure (Pa)
   */
  Real sublimationPressure(Real temperature) const;

  virtual Real vaporPressure(Real temperature) const override;

  virtual void vaporPressure(Real temperature, Real & psat, Real & dpsat_dT) const override;

  /**
   * Saturated liquid density of CO2
   * Valid for temperatures between the triple point temperature
   * and critical temperature
   *
   * Eq. 3.14, from Span and Wagner (reference above)
   *
   * @param temperature CO2 temperature (K)
   * @return saturated liquid density (kg/m^3)
   */
  Real saturatedLiquidDensity(Real temperature) const;

  /**
   * Saturated vapor density of CO2
   * Valid for temperatures between the triple point temperature
   * and critical temperature
   *
   * Eq. 3.15, from Span and Wagner (reference above)
   *
   * @param temperature CO2 temperature (K)
   * @return saturated vapor density (kg/m^3)
   */
  Real saturatedVaporDensity(Real temperature) const;

  virtual Real p_from_rho_T(Real density, Real temperature) const override;

  virtual std::vector<Real> henryCoefficients() const override;

  /**
   * Partial density of dissolved CO2
   * From Garcia, Density of aqueous solutions of CO2, LBNL-49023 (2001)
   *
   * @param temperature fluid temperature (K)
   * @return partial molar density (kg/m^3)
   */
  Real partialDensity(Real temperature) const;

  virtual Real k_from_p_T(Real pressure, Real temperature) const override;

  virtual void
  k_from_p_T(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const override;

  virtual Real k_from_rho_T(Real density, Real temperature) const override;

protected:
  virtual Real alpha(Real delta, Real tau) const override;

  virtual Real dalpha_ddelta(Real delta, Real tau) const override;

  virtual Real dalpha_dtau(Real delta, Real tau) const override;

  virtual Real d2alpha_ddelta2(Real delta, Real tau) const override;

  virtual Real d2alpha_dtau2(Real delta, Real tau) const override;

  virtual Real d2alpha_ddeltatau(Real delta, Real tau) const override;

  /// Molar mass of CO2 (kg/mol)
  const Real _Mco2 = 44.0098e-3;
  /// Critical pressure (Pa)
  const Real _critical_pressure = 7.3773e6;
  /// Critical temperature (K)
  const Real _critical_temperature = 304.1282;
  /// Critical density (kg/m^3)
  const Real _critical_density = 467.6;
  /// Triple point pressure (Pa)
  const Real _triple_point_pressure = 0.51795e6;
  /// Triple point temperature (K)
  const Real _triple_point_temperature = 216.592;
  /// Specific gas constant (J/mol/K)
  const Real _Rco2 = 188.9241;

  /// Coefficients for the ideal gas component of the Helmholtz free energy
  const std::array<Real, 5> _a0{{1.99427042, 0.62105248, 0.41195293, 1.04028922, 0.08327678}};
  const std::array<Real, 5> _theta0{{3.15163, 6.11190, 6.77708, 11.32384, 27.08792}};

  /// Coefficients for the residual component of the Helmholtz free energy
  const std::array<Real, 7> _n1{{0.38856823203161,
                                 2.9385475942740,
                                 -5.5867188534934,
                                 -0.76753199592477,
                                 0.31729005580416,
                                 0.54803315897767,
                                 0.12279411220335}};
  const std::array<unsigned int, 7> _d1{{1, 1, 1, 1, 2, 2, 3}};
  const std::array<Real, 7> _t1{{0.0, 0.75, 1.0, 2.0, 0.75, 2.0, 0.75}};
  const std::array<Real, 27> _n2{
      {2.1658961543220,      1.5841735109724,    -0.23132705405503,   0.058116916431436,
       -0.55369137205382,    0.48946615909422,   -0.024275739843501,  0.062494790501678,
       -0.12175860225246,    -0.37055685270086,  -0.016775879700426,  -0.11960736637987,
       -0.045619362508778,   0.035612789270346,  -0.0074427727132052, -0.0017395704902432,
       -0.021810121289527,   0.024332166559236,  -0.037440133423463,  0.14338715756878,
       -0.13491969083286,    -0.023151225053480, 0.012363125492901,   0.0021058321972940,
       -0.00033958519026368, 0.0055993651771592, -0.00030335118055646}};
  const std::array<unsigned int, 27> _d2{
      {1, 2, 4, 5, 5, 5, 6, 6, 6, 1, 1, 4, 4, 4, 7, 8, 2, 3, 3, 5, 5, 6, 7, 8, 10, 4, 8}};
  const std::array<Real, 27> _t2{{1.5,  1.5,  2.5,  0.0,  1.5,  2.0, 0.0, 1.0,  2.0,
                                  3.0,  6.0,  3.0,  6.0,  8.0,  6.0, 0.0, 7.0,  12.0,
                                  16.0, 22.0, 24.0, 16.0, 24.0, 8.0, 2.0, 28.0, 14.0}};
  const std::array<unsigned int, 27> _c2{
      {1, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 4, 4, 5, 6}};
  const std::array<Real, 5> _n3{
      {-213.65488688320, 26641.569149272, -24027.212204557, -283.41603423999, 212.47284400179}};
  const std::array<unsigned int, 5> _d3{{2, 2, 2, 3, 3}};
  const std::array<unsigned int, 5> _t3{{1, 0, 1, 3, 3}};
  const std::array<Real, 5> _alpha3{{25.0, 25.0, 25.0, 15.0, 20.0}};
  const std::array<Real, 5> _beta3{{325.0, 300.0, 300.0, 275.0, 275.0}};
  const std::array<Real, 5> _gamma3{{1.16, 1.19, 1.19, 1.25, 1.22}};
  const std::array<Real, 5> _eps3{{1.0, 1.0, 1.0, 1.0, 1.0}};
  const std::array<Real, 3> _n4{{-0.66642276540751, 0.72608632349897, 0.055068668612842}};
  const std::array<Real, 3> _a4{{3.5, 3.5, 3.0}};
  const std::array<Real, 3> _b4{{0.875, 0.925, 0.875}};
  const std::array<Real, 3> _beta4{{0.3, 0.3, 0.3}};
  const std::array<Real, 3> _A4{{0.7, 0.7, 0.7}};
  const std::array<Real, 3> _B4{{0.3, 0.3, 1.0}};
  const std::array<Real, 3> _C4{{10.0, 10.0, 12.5}};
  const std::array<Real, 3> _D4{{275.0, 275.0, 275.0}};

  /// Coefficients for viscosity
  const std::array<Real, 5> _mu_a{{0.235156, -0.491266, 5.211155e-2, 5.347906e-2, -1.537102e-2}};
  const std::array<Real, 5> _mu_d{
      {0.4071119e-2, 0.7198037e-4, 0.2411697e-16, 0.2971072e-22, -0.1627888e-22}};

  /// Coefficients for the thermal conductivity
  const std::array<Real, 3> _k_g1{{0.0, 0.0, 1.5}};
  const std::array<Real, 7> _k_g2{{0.0, 1.0, 1.5, 1.5, 1.5, 3.5, 5.5}};
  const std::array<unsigned int, 3> _k_h1{{1, 5, 1}};
  const std::array<unsigned int, 7> _k_h2{{1, 2, 0, 5, 9, 0, 0}};
  const std::array<Real, 3> _k_n1{{7.69857587, 0.159885811, 1.56918621}};
  const std::array<Real, 7> _k_n2{
      {-6.73400790, 16.3890156, 3.69415242, 22.3205514, 66.1420950, -0.171779133, 0.00433043347}};
  const std::array<Real, 12> _k_a{
      {3.0, 6.70697, 0.94604, 0.3, 0.3, 0.39751, 0.33791, 0.77963, 0.79857, 0.9, 0.02, 0.2}};
};

#pragma GCC diagnostic pop
