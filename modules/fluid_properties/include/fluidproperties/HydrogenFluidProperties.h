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
 * Hydrogen (H2) fluid properties as a function of pressure (Pa)
 * and temperature (K).
 *
 * Thermodynamic properties calculated from:
 * Leachman, Jacobsen, Penoncello,and Lemmon, Fundamental equations of state
 * for parahydrogen, normal hydrogen, and orthohydrogen, Journal of Physical
 * and Chemical Reference Data, 38, 721--748 (2009)
 *
 * Viscosity from:
 * Muzny, Huber and Kazakov, Correlation for the viscosity of normal hydrogen
 * obtained from symbolic regression, Journal of Chemical and Engineering Data,
 * 58, 969-979 (2013)
 *
 * Thermal conductivity from:
 * Assael, Assael, Huber, Perkins and Takata, Correlation of the thermal
 * conductivity of normal and parahydrogen from the triple point to 1000 K
 * and up to 100 Mpa, Journal of Physical and Chemical Reference Data, 40 (2011)
 */
class HydrogenFluidProperties : public HelmholtzFluidProperties
{
public:
  static InputParameters validParams();

  HydrogenFluidProperties(const InputParameters & parameters);

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

protected:
  /**
   * Helmholtz free energy for H2
   * From Leachman et al (reference above)
   *
   * @param delta scaled density (-)
   * @param tau scaled temperature (-)
   * @return alpha Helmholtz free energy
   */
  virtual Real alpha(Real delta, Real tau) const override;

  /**
   * Derivative of Helmholtz free energy wrt delta
   *
   * @param delta scaled density (-)
   * @param tau scaled temperature (-)
   * @return derivative of Helmholtz free energy wrt delta
   */
  virtual Real dalpha_ddelta(Real delta, Real tau) const override;

  /**
   * Derivative of Helmholtz free energy wrt tau
   *
   * @param delta scaled density (-)
   * @param tau scaled temperature (-)
   * @return derivative of Helmholtz free energy wrt tau
   */
  virtual Real dalpha_dtau(Real delta, Real tau) const override;

  /**
   * Second derivative of Helmholtz free energy wrt delta
   *
   * @param delta scaled density (-)
   * @param tau scaled temperature (-)
   * @return second derivative of Helmholtz free energy wrt delta
   */
  virtual Real d2alpha_ddelta2(Real delta, Real tau) const override;

  /**
   * Second derivative of Helmholtz free energy wrt tau
   *
   * @param delta scaled density (-)
   * @param tau scaled temperature (-)
   * @return second derivative of Helmholtz free energy wrt tau
   */
  virtual Real d2alpha_dtau2(Real delta, Real tau) const override;

  /**
   * Second derivative of Helmholtz free energy wrt delta and tau
   *
   * @param delta scaled density (-)
   * @param tau scaled temperature (-)
   * @return second derivative of Helmholtz free energy wrt delta and tau
   */
  virtual Real d2alpha_ddeltatau(Real delta, Real tau) const override;

  /// Hydrogen molar mass (kg/mol)
  const Real _Mh2;
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
  const std::array<Real, 5> _a{{1.616, -0.4117, -0.792, 0.758, 1.217}};
  const std::array<Real, 5> _b{
      {-16.0205159149, -22.6580178006, -60.0090511389, -74.9434303817, -206.9392065168}};
  /// Coefficients for residual component of the Helmholtz free energy
  const std::array<Real, 7> _N1{{-6.93643, 0.01, 2.1101, 4.52059, 0.732564, -1.34086, 0.130985}};
  const std::array<Real, 7> _t1{{0.6844, 1.0, 0.989, 0.489, 0.803, 1.1444, 1.409}};
  const std::array<unsigned int, 7> _d1{{1, 4, 1, 1, 2, 2, 3}};

  const std::array<Real, 2> _N2{{-0.777414, 0.351944}};
  const std::array<Real, 2> _t2{{1.754, 1.311}};
  const std::array<unsigned int, 2> _d2{{1, 3}};

  const std::array<Real, 5> _N3{{-0.0211716, 0.0226312, 0.032187, -0.0231752, 0.0557346}};
  const std::array<Real, 5> _t3{{4.187, 5.646, 0.791, 7.249, 2.986}};
  const std::array<unsigned int, 5> _d3{{2, 1, 3, 1, 1}};
  const std::array<Real, 5> _phi3{{-1.685, -0.489, -0.103, -2.506, -1.607}};
  const std::array<Real, 5> _beta3{{-0.171, -0.2245, -0.1304, -0.2785, -0.3967}};
  const std::array<Real, 5> _gamma3{{0.7164, 1.3444, 1.4517, 0.7204, 1.5445}};
  const std::array<Real, 5> _D3{{1.506, 0.156, 1.736, 0.67, 1.662}};

  /// Coefficients for viscosity
  const std::array<Real, 5> _amu{{2.09630e-1, -4.55274e-1, 1.423602e-1, -3.35325e-2, 2.76981e-3}};
  const std::array<Real, 7> _bmu{{-0.187, 2.4871, 3.7151, -11.0972, 9.0965, -3.8292, 0.5166}};
  const std::array<Real, 6> _cmu{
      {6.43449673, 4.56334068e-2, 2.32797868e-1, 9.5832612e-1, 1.27941189e-1, 3.63576595e-1}};

  /// Coefficients for thermal conductivity
  const std::array<Real, 7> _a1k{
      {-3.40976e-1, 4.5882, -1.4508, 3.26394e-1, 3.16939e-3, 1.90592e-4, -1.139e-6}};
  const std::array<Real, 4> _a2k{{1.38497e2, -2.21878e1, 4.57151, 1.0}};
  const std::array<Real, 5> _b1k{{3.63081e-2, -2.07629e-2, 3.1481e-2, -1.43097e-2, 1.7498e-3}};
  const std::array<Real, 5> _b2k{{1.8337e-3, -8.86716e-3, 1.5826e-2, -1.06283e-2, 2.80673e-3}};
};

#pragma GCC diagnostic pop
