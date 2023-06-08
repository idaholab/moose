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
 * Methane (CH4) fluid properties as a function of pressure (Pa)
 * and temperature (K).
 *
 * Thermodynamic properties calculated from:
 * Setzmann and Wagner, A new equation of state and tables of thermodynamic
 * properties for methane covering the range from the melting line to 625 K at
 * pressures up to 100 MPa, Journal of Physical and Chemical Reference Data,
 * 20, 1061--1155 (1991)
 *
 * Viscosity and thermal conductivity calculated from
 * Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas
 * Tables with Computer Equations.
 */
class MethaneFluidProperties : public HelmholtzFluidProperties
{
public:
  static InputParameters validParams();

  MethaneFluidProperties(const InputParameters & parameters);
  virtual ~MethaneFluidProperties();

  virtual std::string fluidName() const override;

  virtual Real molarMass() const override;

  virtual Real mu_from_p_T(Real pressure, Real temperature) const override;

  virtual void mu_from_p_T(
      Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const override;

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
   * Saturated liquid density of CH4
   * Valid for temperatures between the triple point temperature
   * and critical temperature
   *
   * Eq. (3.4), from Setzmann and Wagner (reference above)
   *
   * @param temperature temperature (K)
   * @return saturated liquid density (kg/m^3)
   */
  Real saturatedLiquidDensity(Real temperature) const;

  /**
   * Saturated vapor density of CH4
   * Valid for temperatures between the triple point temperature
   * and critical temperature
   *
   * Eq. (3.5), from Setzmann and Wagner (reference above)
   *
   * @param temperature temperature (K)
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

  /// Methane molar mass (kg/mol)
  const Real _Mch4;
  /// Critical pressure (Pa)
  const Real _p_critical;
  /// Critical temperature (K)
  const Real _T_critical;
  /// Critical density (kg/m^3)
  const Real _rho_critical;
  /// Triple point pressure (Pa)
  const Real _p_triple;
  /// Triple point temperature (K)
  const Real _T_triple;

  /// Coefficients for ideal gas component of the Helmholtz free energy
  const std::array<Real, 5> _a0{{0.008449, 4.6942, 3.4865, 1.6572, 1.4115}};
  const std::array<Real, 5> _b0{{3.4004324, 10.26951575, 20.43932747, 29.93744884, 79.13351945}};

  /// Coefficients for residual component of the Helmholtz free energy
  const std::array<Real, 13> _N1{{0.4367901028e-1,
                                  0.6709236199,
                                  -0.1765577859e1,
                                  0.8582330241,
                                  -0.1206513052e1,
                                  0.512046722,
                                  -0.4000010791e-3,
                                  -0.1247842423e-1,
                                  0.3100269701e-1,
                                  0.1754748522e-2,
                                  -0.3171921605e-5,
                                  -0.224034684e-5,
                                  0.2947056156e-6}};
  const std::array<Real, 13> _t1{
      {-0.5, 0.5, 1.0, 0.5, 1.0, 1.5, 4.5, 0.0, 1.0, 3.0, 1.0, 3.0, 3.0}};
  const std::array<unsigned int, 13> _d1{{1, 1, 1, 2, 2, 2, 2, 3, 4, 4, 8, 9, 10}};

  const std::array<Real, 23> _N2{
      {0.1830487909,     0.1511883679,     -0.4289363877,    0.6894002446e-1, -0.1408313996e-1,
       -0.306305483e-1,  -0.2969906708e-1, -0.1932040831e-1, -0.1105739959,   0.9952548995e-1,
       0.8548437825e-2,  -0.6150555662e-1, -0.4291792423e-1, -0.181320729e-1, 0.344590476e-1,
       -0.238591945e-2,  -0.1159094939e-1, 0.6641693602e-1,  -0.237154959e-1, -0.3961624905e-1,
       -0.1387292044e-1, 0.3389489599e-1,  -0.2927378753e-2}};
  const std::array<Real, 23> _t2{{0.0,  1.0,  2.0,  0.0,  0.0,  2.0,  2.0,  5.0,
                                  5.0,  5.0,  2.0,  4.0,  12.0, 8.0,  10.0, 10.0,
                                  10.0, 14.0, 12.0, 18.0, 22.0, 18.0, 14.0}};
  const std::array<unsigned int, 23> _c2{
      {1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4}};
  const std::array<unsigned int, 23> _d2{
      {1, 1, 1, 2, 4, 5, 6, 1, 2, 3, 4, 4, 3, 5, 5, 8, 2, 3, 4, 4, 4, 5, 6}};

  const std::array<Real, 4> _N3{
      {0.9324799946e-4, -0.6287171518e1, 0.1271069467e2, -0.6423953466e1}};
  const std::array<Real, 4> _t3{{2.0, 0.0, 1.0, 2.0}};
  const std::array<int, 4> _d3{{2, 0, 0, 0}};
  const std::array<Real, 4> _alpha3{{20.0, 40.0, 40.0, 40.0}};
  const std::array<Real, 4> _beta3{{200.0, 250.0, 250.0, 250.0}};
  const std::array<Real, 4> _gamma3{{1.07, 1.11, 1.11, 1.11}};
  const std::array<Real, 4> _D3{{1.0, 1.0, 1.0, 1.0}};

  /// Coefficients for viscosity
  const std::array<Real, 6> _a{
      {2.968267e-1, 3.711201e-2, 1.218298e-5, -7.02426e-8, 7.543269e-11, -2.7237166e-14}};
  /// Coefficients for thermal conductivity
  const std::array<Real, 7> _b{{-1.3401499e-2,
                                3.663076e-4,
                                -1.82248608e-6,
                                5.93987998e-9,
                                -9.1405505e-12,
                                6.7896889e-15,
                                -1.95048736e-18}};
};

#pragma GCC diagnostic pop
