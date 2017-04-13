/****************************************************************/
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*          All contents are licensed under LGPL V2.1           */
/*             See LICENSE for full restrictions                */
/****************************************************************/

#ifndef NACLFLUIDPROPERTIES_H
#define NACLFLUIDPROPERTIES_H

#include "SinglePhaseFluidPropertiesPT.h"

class NaClFluidProperties;

template <>
InputParameters validParams<NaClFluidProperties>();

/**
 * NaCl fluid properties as a function of pressure (Pa) and temperature (K).
 * Note: only solid state (halite) properties are currently implemented to
 * use in brine formulation
 *
 * From Driesner, The system H2O-NaCl. Part II: Correlations for molar volume,
 * enthalpy, and isobaric heat capacity from 0 to 1000 C, 1 to 500 bar, and 0
 * to 1 Xnacl, Geochimica et Cosmochimica Acta 71, 4902-4919 (2007)
 */
class NaClFluidProperties : public SinglePhaseFluidPropertiesPT
{
public:
  NaClFluidProperties(const InputParameters & parameters);
  virtual ~NaClFluidProperties();

  /**
   * NaCl molar mass
   * @return molar mass (kg/mol)
   */
  virtual Real molarMass() const override;

  /**
   * NaCl critical pressure
   * From Anderko and Pitzer, Equation of state for pure sodium chloride, Fluid
   * Phase Equil., 79 (1992)
   * @return critical pressure (Pa)
   */
  virtual Real criticalPressure() const;

  /**
   * NaCl critical temperature
   * From Anderko and Pitzer, Equation of state for pure sodium chloride, Fluid
   * Phase Equil., 79 (1992)
   * @return critical temperature (K)
   */
  virtual Real criticalTemperature() const;

  /**
   * NaCl critical density
   * From Anderko and Pitzer, Equation of state for pure sodium chloride, Fluid
   * Phase Equil., 79 (1992)
   * @return critical density (kg/m^3)
   */
  virtual Real criticalDensity() const;

  /**
   * NaCl density as a function of pressure and temperature
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return density (kg/m^3)
   */
  virtual Real rho(Real pressure, Real temperature) const override;

  /**
   * NaCl density as a function of pressure and temperature, and
   * derivatives wrt pressure and temperature
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
   * Isobaric specific heat capacity
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
   * NaCl viscosity
   *
   * @param density fluid density (kg/m^3)
   * @param temperature fluid temperature (K)
   * @return viscosity (Pa.s)
   */
  virtual Real mu(Real density, Real temperature) const override;

  /**
   * NaCl viscosity and derivatives wrt density and temperature
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
   * Thermal conductivity as a function of pressure and temperature
   * From Urqhart and Bauer, Experimental determination of single-crystal halite
   * thermal conductivity, diffusivity and specific heat from -75 C to 300 C,
   * Int. J. Rock Mech. and Mining Sci., 78 (2015)
   * Note: The function given in this reference doesn't satisfactorily match their
   * experimental data, so the data was refitted using a third order polynomial
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return k (W/m/K)
   */
  virtual Real k(Real pressure, Real temperature) const override;

  /**
   * Specific entropy as a function of pressure and temperature
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return s (J/kg/K)
   */
  virtual Real s(Real pressure, Real temperature) const override;

  /**
   * Enthalpy as a function of pressure and temperature
   *
   * @param pressure fluid pressure (Pa)
   * @param temperature fluid temperature (K)
   * @return h (J/kg)
   */
  virtual Real h(Real pressure, Real temperature) const override;

  /**
   * Enthalpy and its derivatives wrt pressure and temperature.
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
   * Henry's law constant
   * Note: not implemented in this fluid property
   */
  virtual Real henryConstant(Real temperature) const override;

  /**
   * Henry's law constant and derivative wrt temperature
   * Note: not implemented in this fluid property
   */
  virtual void henryConstant_dT(Real temperature, Real & Kh, Real & dKh_dT) const override;

protected:
  /// NaCl molar mass (kg/mol)
  const Real _Mnacl;
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
};

#endif /* NACLFLUIDPROPERTIES_H */
