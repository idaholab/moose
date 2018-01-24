//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef CO2FLUIDPROPERTIES_H
#define CO2FLUIDPROPERTIES_H

#include "SinglePhaseFluidPropertiesPT.h"

class CO2FluidProperties;

template <>
InputParameters validParams<CO2FluidProperties>();

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
class CO2FluidProperties : public SinglePhaseFluidPropertiesPT
{
public:
  CO2FluidProperties(const InputParameters & parameters);
  virtual ~CO2FluidProperties();

  virtual Real rho(Real pressure, Real temperature) const override;

  virtual void rho_dpT(
      Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const override;

  virtual Real mu(Real pressure, Real temperature) const override;

  virtual void
  mu_dpT(Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const override;

  virtual Real mu_from_rho_T(Real density, Real temperature) const override;

  virtual void mu_drhoT_from_rho_T(Real density,
                                   Real temperature,
                                   Real ddensity_dT,
                                   Real & mu,
                                   Real & dmu_drho,
                                   Real & dmu_dT) const override;

  virtual std::string fluidName() const override;

  Real molarMass() const override;

  /**
   * CO2 critical pressure
   * @return critical pressure (Pa)
   */
  Real criticalPressure() const;

  /**
   * CO2 critical temperature
   * @return critical temperature (K)
   */
  Real criticalTemperature() const;

  /**
   * CO2 critical density
   * @return critical density (kg/m^3)
   */
  Real criticalDensity() const;

  /**
   * CO2 triple point pressure
   * @return triple point pressure (Pa)
   */
  Real triplePointPressure() const;

  /**
   * CO2 triple point temperature
   * @return triple point temperature (K)
   */
  Real triplePointTemperature() const;

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

  /**
   * Vapor pressure. Used to delineate liquid and gas phases.
   * Valid for temperatures between the triple point temperature
   * and the critical temperature
   *
   * Eq. 3.13, from Span and Wagner (reference above)
   *
   * @param temperature CO2 temperature (K)
   * @return vapor pressure (Pa)
   */
  Real vaporPressure(Real temperature) const;

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

  /**
   * Pressure as a function of density and temperature
   * From Span and Wagner (reference above)
   *
   * @param density CO2 density (kg/m^3)
   * @param temperature CO2 temperature (K)
   * @return CO2 pressure (Pa)
   */
  Real pressure(Real density, Real temperature) const;

  /**
   * Internal function to calculate pressure as a function of density and
   * temperature using the Span and Wagner EOS. This function is called by
   * pressure(density, temperature)
   *
   * @param density CO2 density (kg/m^3)
   * @param temperature CO2 temperature (K)
   * @return CO2 pressure (Pa)
   */
  Real pressureSW(Real density, Real temperature) const;

  /**
   * Helmholtz free energy for CO2
   * From Span and Wagner (reference above)
   *
   * @param delta scaled density (-)
   * @param tau scaled temperature (-)
   * @return phi Helmholtz free energy
   */
  Real phiSW(Real delta, Real tau) const;

  /**
   * Derivative of Helmholtz free energy wrt delta
   *
   * @param delta scaled density (-)
   * @param tau scaled temperature (-)
   * @return derivative of Helmholtz free energy wrt delta
   */
  Real dphiSW_dd(Real delta, Real tau) const;

  /**
   * Derivative of Helmholtz free energy wrt tau
   *
   * @param delta scaled density (-)
   * @param tau scaled temperature (-)
   * @return derivative of Helmholtz free energy wrt tau
   */
  Real dphiSW_dt(Real delta, Real tau) const;

  /**
   * Second derivative of Helmholtz free energy wrt delta
   *
   * @param delta scaled density (-)
   * @param tau scaled temperature (-)
   * @return second derivative of Helmholtz free energy wrt delta
   */
  Real d2phiSW_dd2(Real delta, Real tau) const;

  /**
   * Second derivative of Helmholtz free energy wrt tau
   *
   * @param delta scaled density (-)
   * @param tau scaled temperature (-)
   * @return second derivative of Helmholtz free energy wrt tau
   */
  Real d2phiSW_dt2(Real delta, Real tau) const;

  /**
   * Second derivative of Helmholtz free energy wrt delta and tau
   *
   * @param delta scaled density (-)
   * @param tau scaled temperature (-)
   * @return second derivative of Helmholtz free energy wrt delta and tau
   */
  Real d2phiSW_ddt(Real delta, Real tau) const;

  virtual Real henryConstant(Real temperature) const override;

  virtual void henryConstant_dT(Real temperature, Real & Kh, Real & dKh_dT) const override;

  /**
   * Partial density of dissolved CO2
   * From Garcia, Density of aqueous solutions of CO2, LBNL-49023 (2001)
   *
   * @param temperature fluid temperature (K)
   * @return partial molar density (kg/m^3)
   */
  Real partialDensity(Real temperature) const;

  virtual Real e(Real pressure, Real temperature) const override;

  virtual void
  e_dpT(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const override;

  virtual void rho_e_dpT(Real pressure,
                         Real temperature,
                         Real & rho,
                         Real & drho_dp,
                         Real & drho_dT,
                         Real & e,
                         Real & de_dp,
                         Real & de_dT) const override;

  virtual Real c(Real pressure, Real temperature) const override;

  virtual Real cp(Real pressure, Real temperature) const override;

  virtual Real cv(Real pressure, Real temperature) const override;

  virtual Real k(Real pressure, Real temperature) const override;

  virtual void
  k_dpT(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const override;

  virtual Real k_from_rho_T(Real density, Real temperature) const override;

  virtual Real s(Real pressure, Real temperature) const override;

  virtual Real h(Real p, Real T) const override;

  virtual void
  h_dpT(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const override;

  virtual Real beta(Real pressure, Real temperature) const override;

protected:
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
  std::vector<Real> _a0{1.99427042, 0.62105248, 0.41195293, 1.04028922, 0.08327678};
  std::vector<Real> _theta0{3.15163, 6.11190, 6.77708, 11.32384, 27.08792};

  /// Coefficients for the residual component of the Helmholtz free energy
  std::vector<Real> _n1{0.38856823203161,
                        2.9385475942740,
                        -5.5867188534934,
                        -0.76753199592477,
                        0.31729005580416,
                        0.54803315897767,
                        0.12279411220335};
  std::vector<Real> _d1{1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 3.0};
  std::vector<Real> _t1{0.0, 0.75, 1.0, 2.0, 0.75, 2.0, 0.75};
  std::vector<Real> _n2{
      2.1658961543220,      1.5841735109724,    -0.23132705405503,   0.058116916431436,
      -0.55369137205382,    0.48946615909422,   -0.024275739843501,  0.062494790501678,
      -0.12175860225246,    -0.37055685270086,  -0.016775879700426,  -0.11960736637987,
      -0.045619362508778,   0.035612789270346,  -0.0074427727132052, -0.0017395704902432,
      -0.021810121289527,   0.024332166559236,  -0.037440133423463,  0.14338715756878,
      -0.13491969083286,    -0.023151225053480, 0.012363125492901,   0.0021058321972940,
      -0.00033958519026368, 0.0055993651771592, -0.00030335118055646};
  std::vector<Real> _d2{1.0, 2.0, 4.0, 5.0, 5.0, 5.0, 6.0, 6.0, 6.0, 1.0, 1.0,  4.0, 4.0, 4.0,
                        7.0, 8.0, 2.0, 3.0, 3.0, 5.0, 5.0, 6.0, 7.0, 8.0, 10.0, 4.0, 8.0};
  std::vector<Real> _t2{1.5, 1.5, 2.5, 0.0,  1.5,  2.0,  0.0,  1.0,  2.0,  3.0, 6.0, 3.0,  6.0, 8.0,
                        6.0, 0.0, 7.0, 12.0, 16.0, 22.0, 24.0, 16.0, 24.0, 8.0, 2.0, 28.0, 14.0};
  std::vector<Real> _c2{1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 2.0, 2.0, 2.0, 2.0, 2.0,
                        2.0, 2.0, 3.0, 3.0, 3.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 5.0, 6.0};
  std::vector<Real> _n3{
      -213.65488688320, 26641.569149272, -24027.212204557, -283.41603423999, 212.47284400179};
  std::vector<Real> _d3{2.0, 2.0, 2.0, 3.0, 3.0};
  std::vector<Real> _t3{1.0, 0.0, 1.0, 3.0, 3.0};
  std::vector<Real> _alpha3{25.0, 25.0, 25.0, 15.0, 20.0};
  std::vector<Real> _beta3{325.0, 300.0, 300.0, 275.0, 275.0};
  std::vector<Real> _gamma3{1.16, 1.19, 1.19, 1.25, 1.25};
  std::vector<Real> _eps3{1.0, 1.0, 1.0, 1.0, 1.0};
  std::vector<Real> _n4{-0.66642276540751, 0.72608632349897, 0.055068668612842};
  std::vector<Real> _a4{3.5, 3.5, 3.5};
  std::vector<Real> _b4{0.875, 0.925, 0.875};
  std::vector<Real> _beta4{0.3, 0.3, 0.3};
  std::vector<Real> _A4{0.7, 0.7, 0.7};
  std::vector<Real> _B4{0.3, 0.3, 1.0};
  std::vector<Real> _C4{10.0, 10.0, 12.5};
  std::vector<Real> _D4{275.0, 275.0, 275.0};
};

#endif /* CO2FLUIDPROPERTIES_H */
