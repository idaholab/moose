#ifndef METHANEFLUIDPROPERTIES_H
#define METHANEFLUIDPROPERTIES_H

#include "SinglePhaseFluidPropertiesPT.h"

class MethaneFluidProperties;

template<>
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
 virtual Real molarMass() const;

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
  virtual Real rho(Real pressure, Real temperature) const;

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
  virtual void rho_dpT(Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT) const;

  /**
   * Internal energy from pressure and temperature
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return internal enerygy (kJ/kg)
   */
  virtual Real e(Real pressure, Real temperature) const;

  /**
   * Internal energy and its derivatives wrt pressure and temperature
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] e internal energy (kJ/kg)
   * @param[out] de_dp derivative of internal energy wrt pressure
   * @param[out] de_dT derivative of internal energy wrt temperature
   */
  virtual void e_dpT(Real pressure, Real temperature, Real & e, Real & de_dp, Real & de_dT) const;

  /**
   * Density and internal energy and their derivatives wrt pressure and temperature
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] rho density (kg/m^3)
   * @param[out] drho_dp derivative of density wrt pressure
   * @param[out] drho_dT derivative of density wrt temperature
   * @param[out] e internal energy (kJ/kg)
   * @param[out] de_dp derivative of internal energy wrt pressure
   * @param[out] de_dT derivative of internal energy wrt temperature
   */
  virtual void rho_e_dpT(Real pressure, Real temperature, Real & rho, Real & drho_dp, Real & drho_dT, Real & e, Real & de_dp, Real & de_dT) const;

  /**
   * Speed of sound
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return speed of sound (m/s)
   */
  virtual Real c(Real pressure, Real temperature) const;

  /**
   * Isobaric specific heat capacity as a function of pressure and temperature.
   * From Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
   * Computer Equations.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return cp (kJ/kg/K)
   */
  virtual Real cp(Real pressure, Real temperature) const;

  /**
   * Isochoric specific heat
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return cv (kJ/kg/K)
   */
  virtual Real cv(Real pressure, Real temperature) const;

  /**
   * Methane gas viscosity as a function of density and temperature.
   * From Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
   * Computer Equations.
   *
   * @param density fluid density (kg/m^3)
   * @param temperature fluid temperature (K)
   * @return viscosity (Pa.s)
   */
  virtual Real mu(Real density, Real temperature) const;

  /**
   * Methane gas viscosity and derivatives wrt density and temperature.
   * From Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
   * Computer Equations.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] mu viscosity (Pa.s)
   * @param[out] dmu_drho derivative of viscosity wrt density
   * @param[out] dmu_dT derivative of viscosity wrt temperature
   */
  virtual void mu_drhoT(Real denisty, Real temperature, Real & mu, Real & dmu_drho, Real & dmu_dT) const;

  /**
   * Thermal conductivity as a function of pressure and temperature.
   * From Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
   * Computer Equations.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return k (W/m/K)
   */
  virtual Real k(Real pressure, Real temperature) const;

  /**
   * Specific entropy as a function of pressure and temperature.
   * From Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
   * Computer Equations.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return s (kJ/kg/K)
   */
  virtual Real s(Real pressure, Real temperature) const;

  /**
   * Enthalpy as a function of pressure and temperature.
   * From Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
   * Computer Equations.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return h (kJ/kg)
   */
  virtual Real h(Real pressure, Real temperature) const;

  /**
   * Enthalpy and its derivatives wrt pressure and temperature.
   * From Irvine Jr, T. F. and Liley, P. E. (1984) Steam and Gas Tables with
   * Computer Equations.
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @param[out] h (kJ/kg)
   * @param[out] dh_dp derivative of enthalpy wrt pressure
   * @param[out] dh_dT derivative of enthalpy wrt temperature
   */
  virtual void h_dpT(Real pressure, Real temperature, Real & h, Real & dh_dp, Real & dh_dT) const;

  /**
   * Thermal expansion coefficient
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return beta (1/K)
   */
  virtual Real beta(Real pressure, Real temperature) const;

  /**
   * Henry's law constant coefficients for dissolution of CH4 into water.
   * From Guidelines on the Henry's constant and vapour
   * liquid distribution constant for gases in H20 and D20 at high
   * temperatures, IAPWS (2004).
   *
   * @return constants for Henry's constant (-)
   */
  std::vector<Real> henryConstants();

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
