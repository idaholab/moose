//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#ifndef METHANEFLUIDPROPERTIES_H
#define METHANEFLUIDPROPERTIES_H

#include "SinglePhaseFluidPropertiesPT.h"
#include <array>

class MethaneFluidProperties;

template <>
InputParameters validParams<MethaneFluidProperties>();

/**
 * Methane (CH4) fluid properties as a function of pressure (Pa)
 * and temperature (K).
 *
 * Density is computed assuming an ideal gas.
 * Viscosity, enthalpy, entropy, thermal conductivity and specific heat calculated from
 * Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas
 * Tables with Computer Equations.
 */
class MethaneFluidProperties : public SinglePhaseFluidPropertiesPT
{
public:
  MethaneFluidProperties(const InputParameters & parameters);
  virtual ~MethaneFluidProperties();

  virtual std::string fluidName() const override;

  virtual Real molarMass() const override;

  virtual Real rho_from_p_T(Real pressure, Real temperature) const override;

  virtual void rho_from_p_T(
      Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const override;

  virtual Real e_from_p_T(Real pressure, Real temperature) const override;

  virtual void
  e_from_p_T(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const override;

  virtual void rho_e_dpT(Real pressure,
                         Real temperature,
                         Real & rho,
                         Real & drho_dp,
                         Real & drho_dT,
                         Real & e,
                         Real & de_dp,
                         Real & de_dT) const override;

  virtual Real c_from_p_T(Real pressure, Real temperature) const override;

  virtual Real cp_from_p_T(Real pressure, Real temperature) const override;

  virtual Real cv_from_p_T(Real pressure, Real temperature) const override;

  virtual Real mu_from_p_T(Real pressure, Real temperature) const override;

  virtual void mu_from_p_T(
      Real pressure, Real temperature, Real & mu, Real & dmu_dp, Real & dmu_dT) const override;

  virtual void rho_mu(Real pressure, Real temperature, Real & rho, Real & mu) const override;

  virtual void rho_mu_dpT(Real pressure,
                          Real temperature,
                          Real & rho,
                          Real & drho_dp,
                          Real & drho_dT,
                          Real & mu,
                          Real & dmu_dp,
                          Real & dmu_dT) const override;

  virtual Real k_from_p_T(Real pressure, Real temperature) const override;

  virtual void
  k_from_p_T(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const override;

  virtual Real s_from_p_T(Real pressure, Real temperature) const override;
  virtual void s_from_p_T(Real p, Real T, Real & s, Real & ds_dp, Real & ds_dT) const override;

  virtual Real h_from_p_T(Real pressure, Real temperature) const override;

  virtual void
  h_from_p_T(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const override;

  virtual Real henryConstant(Real temperature) const override;

  virtual void henryConstant_dT(Real temperature, Real & Kh, Real & dKh_dT) const override;

  virtual Real criticalPressure() const override;

  virtual Real criticalTemperature() const override;

  virtual Real criticalDensity() const override;

  virtual Real triplePointPressure() const override;

  virtual Real triplePointTemperature() const override;

protected:
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

  /// Coefficients for enthalpy, cp etc
  const std::array<Real, 7> _a0 = {
      {1.9165258, -1.09269e-3, 8.696605e-6, -5.2291144e-9, 0.0, 0.0, 0.0}};
  const std::array<Real, 7> _a1 = {
      {1.04356e1, -4.2025284e-2, 8.849006e-5, -8.4304566e-8, 3.9030203e-11, -7.1345169e-15, 0.0}};
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

#endif /* METHANEFLUIDPROPERTIES_H */
