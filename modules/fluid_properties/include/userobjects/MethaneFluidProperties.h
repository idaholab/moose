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

  virtual Real rho(Real pressure, Real temperature) const override;

  virtual void rho_dpT(
      Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const override;

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

  virtual Real k(Real pressure, Real temperature) const override;

  virtual void
  k_dpT(Real pressure, Real temperature, Real & k, Real & dk_dp, Real & dk_dT) const override;

  virtual Real k_from_rho_T(Real density, Real temperature) const override;

  virtual Real s(Real pressure, Real temperature) const override;

  virtual Real h(Real pressure, Real temperature) const override;

  virtual void
  h_dpT(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const override;

  virtual Real beta(Real pressure, Real temperature) const override;

  virtual Real henryConstant(Real temperature) const override;

  virtual void henryConstant_dT(Real temperature, Real & Kh, Real & dKh_dT) const override;

  /**
   * Methane critical pressure
   * @return critical pressure (Pa)
   */
  virtual Real criticalPressure() const;

  /**
   * Methane critical temperature
   * @return critical temperature (K)
   */
  virtual Real criticalTemperature() const;

  /**
   * Methane critical density
   * @return critical density (kg/m^3)
   */
  virtual Real criticalDensity() const;

protected:
  /// Methane molar mass (kg/mol)
  const Real _Mch4;
  /// Critical pressure (Pa)
  const Real _p_critical;
  /// Critical temperature (K)
  const Real _T_critical;
  /// Critical density (kg/m^3)
  const Real _rho_critical;
  /// Coefficient of thermal expansion (1/K)
  const Real _beta;
};

#endif /* METHANEFLUIDPROPERTIES_H */
