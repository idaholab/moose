/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef METHANEFLUIDPROPERTIES_H
#define METHANEFLUIDPROPERTIES_H

#include "SinglePhaseFluidPropertiesPT.h"

class MethaneFluidProperties;

template <>
InputParameters validParams<MethaneFluidProperties>();

/**
 * Methane (CH4) fluid properties as a function of pressure (Pa)
 * and temperature (K)
 */
class MethaneFluidProperties : public SinglePhaseFluidPropertiesPT
{
public:
  MethaneFluidProperties(const InputParameters & parameters);
  virtual ~MethaneFluidProperties();

  /**
   * Methane molar mass
   * @return molar mass (kg/mol)
   */
  virtual Real molarMass() const override;

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

  /**
   * Methane gas density as a function of pressure and temperature
   * (assuming an ideal gas)
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return density (kg/m^3)
   */
  virtual Real rho(Real pressure, Real temperature) const override;

  /**
   * Methane gas density as a function of pressure and temperature, and
   * derivatives wrt pressure and temperature (assuming an ideal gas)
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] rho density (kg/m^3)
   * @param[out] drho_dp derivative of density wrt pressure
   * @param[out] drho_dT derivative of density wrt temperature
   */
  virtual void rho_dpT(
      Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const override;

  /**
   * Internal energy from pressure and temperature
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return internal enerygy (J/kg)
   */
  virtual Real e(Real pressure, Real temperature) const override;

  /**
   * Internal energy and its derivatives wrt pressure and temperature
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] e internal energy (J/kg)
   * @param[out] de_dp derivative of internal energy wrt pressure
   * @param[out] de_dT derivative of internal energy wrt temperature
   */
  virtual void
  e_dpT(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const override;

  /**
   * Density and internal energy and their derivatives wrt pressure and temperature
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] rho density (kg/m^3)
   * @param[out] drho_dp derivative of density wrt pressure
   * @param[out] drho_dT derivative of density wrt temperature
   * @param[out] e internal energy (J/kg)
   * @param[out] de_dp derivative of internal energy wrt pressure
   * @param[out] de_dT derivative of internal energy wrt temperature
   */
  virtual void rho_e_dpT(Real pressure,
                         Real temperature,
                         Real & rho,
                         Real & drho_dp,
                         Real & drho_dT,
                         Real & e,
                         Real & de_dp,
                         Real & de_dT) const override;

  /**
   * Speed of sound
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return speed of sound (m/s)
   */
  virtual Real c(Real pressure, Real temperature) const override;

  /**
   * Isobaric specific heat capacity as a function of pressure and temperature.
   * From Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
   * Computer Equations.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return cp (J/kg/K)
   */
  virtual Real cp(Real pressure, Real temperature) const override;

  /**
   * Isochoric specific heat
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return cv (J/kg/K)
   */
  virtual Real cv(Real pressure, Real temperature) const override;

  /**
   * Methane gas viscosity as a function of density and temperature.
   * From Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
   * Computer Equations.
   *
   * @param density fluid density (kg/m^3)
   * @param temperature fluid temperature (K)
   * @return viscosity (Pa.s)
   */
  virtual Real mu(Real density, Real temperature) const override;

  /**
   * Methane gas viscosity and derivatives wrt density and temperature.
   * From Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
   * Computer Equations.
   *
   * @param density fluid density (kg/m^3)
   * @param temperature fluid temperature (K)
   * @param[out] mu viscosity (Pa.s)
   * @param[out] dmu_drho derivative of viscosity wrt density
   * @param[out] dmu_dT derivative of viscosity wrt temperature
   */
  virtual void mu_drhoT(
      Real density, Real temperature, Real & mu, Real & dmu_drho, Real & dmu_dT) const override;

  /**
   * Thermal conductivity as a function of pressure and temperature.
   * From Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
   * Computer Equations.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return k (W/m/K)
   */
  virtual Real k(Real pressure, Real temperature) const override;

  /**
   * Specific entropy as a function of pressure and temperature.
   * From Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
   * Computer Equations.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return s (J/kg/K)
   */
  virtual Real s(Real pressure, Real temperature) const override;

  /**
   * Enthalpy as a function of pressure and temperature.
   * From Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
   * Computer Equations.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return h (J/kg)
   */
  virtual Real h(Real pressure, Real temperature) const override;

  /**
   * Enthalpy and its derivatives wrt pressure and temperature.
   * From Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
   * Computer Equations.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] h (J/kg)
   * @param[out] dh_dp derivative of enthalpy wrt pressure
   * @param[out] dh_dT derivative of enthalpy wrt temperature
   */
  virtual void
  h_dpT(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const override;

  /**
   * Thermal expansion coefficient
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return beta (1/K)
   */
  virtual Real beta(Real pressure, Real temperature) const override;

  /**
   * Henry's law constant for dissolution of CH4 into water.
   * From Guidelines on the Henry's constant and vapour
   * liquid distribution constant for gases in H20 and D20 at high
   * temperatures, IAPWS (2004).
   *
   * @param temperature fluid temperature (K)
   * @return Henry's constant (Pa)
   */
  virtual Real henryConstant(Real temperature) const override;

  /**
   * Henry's law constant for dissolution of CH4 into water and
   * derivative wrt temperature.
   * From Guidelines on the Henry's constant and vapour
   * liquid distribution constant for gases in H20 and D20 at high
   * temperatures, IAPWS (2004).
   *
   * @param temperature fluid temperature (K)
   * @param[out] Kh Henry's constant (Pa)
   * @param[out] dKh_dT derivative of Henry's constant wrt temperature
   */
  virtual void henryConstant_dT(Real temperature, Real & Kh, Real & dKh_dT) const override;

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
